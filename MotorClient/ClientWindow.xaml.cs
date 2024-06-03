﻿using MotorServer;
using System;
using System.Collections.Generic;
using System.Diagnostics.Eventing.Reader;
using System.Linq;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace MotorClient
{
    /// <summary>
    /// Interaction logic for ClientWindow.xaml
    /// </summary>
    public partial class ClientWindow : Window
    {
        private bool debug = false;

        private TcpClient clientSocket;
        private NetworkStream networkStream;

        private bool listening = false;
        private string cmdOutput;

        private List<string> cmdQueue;
        public ClientWindow()
        {
            cmdOutput = "";
            cmdQueue = new List<string>();

            InitializeComponent();
            RefreshCmdOutput();


            if (debug) return;

            int response = ConnectToServer();
            if (response == 0)
            {

                listening = true;
                ListenToServer();


                SendCommandsFromQueue();
            }
        }

        #region CONNECTION
        //private static string serverIPv4 = "188.2.24.251";
        private static string serverIPv4 = "192.168.0.100";
        private int serverPort = 12345;

        private int ConnectToServer()
        {
            try
            {
                clientSocket = new TcpClient(serverIPv4, serverPort);
                networkStream = clientSocket.GetStream();
                return clientSocket == null ? 1 : 0;
            }
            catch
            {
                return 1;
            }
        }
        private int DisconnectFromServer()
        {
            if (debug) return 0;
            SendCommand("quit");
            if (clientSocket != null)
                clientSocket.Close();
            return 0;
        }
        private bool sending = false;
        private void SendCommand(string cmd)
        {
            cmdQueue.Add(cmd);
        }
        private void StartSendCmdLoop()
        {
            var sendCmdLoop = SendCommandsFromQueue();
            sendCmdLoop.RunSynchronously();
        }
        private async Task<int> SendCommandsFromQueue()
        {
            while (true)
            {
                try
                {
                    if (debug)
                    {
                        continue;
                    }

                    while (cmdQueue.Count > 0)
                    {
                        string command = cmdQueue[0];
                        cmdQueue.RemoveAt(0);

                        await Task.Delay(100);
                        byte[] sendBytes = Encoding.ASCII.GetBytes(command);
                        await networkStream.WriteAsync(sendBytes, 0, sendBytes.Length);
                        networkStream.Flush();
                        AddLineToCmdOutput($"<<CLIENT>>\tSent command \"{command}\" to server.");
                    }

                }
                catch (Exception ex)
                {
                    await Console.Out.WriteLineAsync(ex.Message);
                    continue;
                }

                await Task.Delay(10);
            }
        }
        private async Task<int> RecieveResponce()
        {
            try
            {
                if (debug)
                {
                    AddLineToCmdOutput("Recieved mock responce");
                    return 0;
                }
                byte[] bytesFrom = new byte[1024];
                await networkStream.ReadAsync(bytesFrom, 0, bytesFrom.Length);
                string dataFromServer = Encoding.ASCII.GetString(bytesFrom);
                dataFromServer = dataFromServer.Split('\0')[0];

                Console.WriteLine("Server response: " + dataFromServer);
                AddLineToCmdOutput(dataFromServer);

                return dataFromServer.Length > 0 ? 0 : 1;
            }
            catch (NullReferenceException nrex)
            {
                await Console.Out.WriteLineAsync(nrex.Message);
                return 1;
            }
            catch (Exception ex)
            {
                await Console.Out.WriteLineAsync(ex.Message);
                return 2;
            }
        }
        private async Task ListenToServer()
        {
            while (listening)
            {
                RecieveResponce();
                await Task.Delay(500);
            }

        }
        #endregion

        #region INTERACTIONS
        private void btn_quit_Click(object sender, RoutedEventArgs e)
        {
            DisconnectFromServer();
            Close();
        }

        private void btn_cmd1_Click(object sender, RoutedEventArgs e)
        {
            //AddLineToCmdOutput("CLIENT: Sending rotate clockwise");
            string cmd = "RCW";
            float speed = 0;
            float.TryParse(tb_speed_cw.Text, out speed);
            float duration = 0;
            float.TryParse(tb_duration_cw.Text, out duration);
            SendCommand($"{cmd} {speed} {duration}");
        }
        private void btn_cmd2_Click(object sender, RoutedEventArgs e)
        {
            //AddLineToCmdOutput("CLIENT: Sending rotate counter clockwise");
            string cmd = "RCCW";
            float speed = 0;
            float.TryParse(tb_speed_ccw.Text, out speed);
            float duration = 0;
            float.TryParse(tb_duration_ccw.Text, out duration);
            SendCommand($"{cmd} {speed} {duration}");
        }

        #endregion

        #region WINDOW
        private string GetDateTime()
        {
            return DateTime.Now.ToString() + ":  ";
        }
        private void AddLineToCmdOutput(string line = "")
        {
            cmdOutput = GetDateTime() + line + '\n' + cmdOutput;
            RefreshCmdOutput();
        }
        private void RefreshCmdOutput()
        {
            consoleOutput.Content = cmdOutput;
        }

        #endregion

        private void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            DisconnectFromServer();
            listening = false;
        }


        private void btn_addCwCmd_Click(object sender, RoutedEventArgs e)
        {
            lv_komande.Items.Add(new CommandView("Rotiraj CW", tb_speed_cw.Text, tb_duration_cw.Text + " ms"));
        }

        private void btn_addCcwCmd_Click(object sender, RoutedEventArgs e)
        {
            lv_komande.Items.Add(new CommandView("Rotiraj CCW", tb_speed_ccw.Text, tb_duration_ccw.Text + " ms"));
        }

        private void btn_sendCommands_Click(object sender, RoutedEventArgs e)
        {
            ProcessCommandsFromList();
        }
        private async Task ProcessCommandsFromList()
        {
            foreach (CommandView item in lv_komande.Items)
            {
                string cmd = "";
                if (item.Komanda == "Rotiraj CW")
                    cmd += "RCW ";
                else
                    cmd += "RCCW ";

                cmd += item.Brzina + " ";
                cmd += item.TrajanjeVal;
                SendCommand(cmd);
                //sendTask.Start();
            }
            lv_komande.Items.Clear();
        }

        private void btn_removeCommand_Click(object sender, RoutedEventArgs e)
        {
            if (lv_komande.Items.Count == 0) return;
            if (lv_komande.SelectedItem != null)
            {
                lv_komande.Items.RemoveAt(lv_komande.SelectedIndex);
                lv_komande.SelectedItem = null;
            }
            else
            {
                lv_komande.Items.RemoveAt(lv_komande.Items.Count - 1);
            }
        }
    }
}
