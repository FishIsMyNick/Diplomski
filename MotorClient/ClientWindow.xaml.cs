using MotorServer;
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

        private TcpClient clientMessageSocket, clientSpeedSocket;
        private NetworkStream messageStream, speedStream;

        private bool listening = false;
        private string cmdOutput;
        private string spdOutput;

        private bool measureSpeed = false;

        private SolidColorBrush greenColor = new SolidColorBrush(Colors.Green);
        private SolidColorBrush redColor = new SolidColorBrush(Colors.Red);

        private List<string> cmdQueue;
        public ClientWindow()
        {
            cmdOutput = "";
            cmdQueue = new List<string>();

            spdOutput = "ISKLJUČENO";

            InitializeComponent();
            RefreshCmdOutput();
            RefreshSpdOutput();


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
        private int messagePort = 12345, speedPort = 12346;

        private int ConnectToServer()
        {
            try
            {
                clientMessageSocket = new TcpClient(serverIPv4, messagePort);
                clientSpeedSocket = new TcpClient(serverIPv4, speedPort);
                messageStream = clientMessageSocket.GetStream();
                speedStream = clientSpeedSocket.GetStream();
                return clientMessageSocket == null || clientSpeedSocket == null ? 1 : 0;
            }
            catch
            {
                return 1;
            }
        }
        private bool quitComplete = false;
        private async Task<int> DisconnectFromServer()
        {
            if (debug) return 0;

            SendCommand("quit");
            await Task.Delay(1000);

            if (clientMessageSocket != null)
                clientMessageSocket.Close();

            quitComplete = true;
            return 0;
        }
        private bool sending = false;
        private void SendCommand(string cmd)
        {
            cmdQueue.Add(cmd);
        }
        private async Task QuitServer()
        {
            try
            {
                string command = "quit";

                await Task.Delay(100);
                byte[] sendBytes = Encoding.ASCII.GetBytes(command);
                await messageStream.WriteAsync(sendBytes, 0, sendBytes.Length);
                messageStream.Flush();
                AddLineToCmdOutput($"<<CLIENT>>\tSent command \"{command}\" to server.");
            }
            catch (Exception ex)
            {
                await Console.Out.WriteLineAsync(ex.Message);
            }
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
                        await messageStream.WriteAsync(sendBytes, 0, sendBytes.Length);
                        messageStream.Flush();
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
        private async Task<int> RecieveMessages()
        {
            try
            {
                if (debug)
                {
                    AddLineToCmdOutput("Recieved mock responce");
                    return 0;
                }
                byte[] bytesFrom = new byte[1024];
                await messageStream.ReadAsync(bytesFrom, 0, bytesFrom.Length);
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
        private async Task<int> RecieveSpeed()
        {
            try
            {
                if (debug)
                {
                    AddLineToSpdOutput("Recieved mock speed");
                    return 0;
                }
                if (measureSpeed)
                {
                    byte[] bytesFrom = new byte[1024];
                    await speedStream.ReadAsync(bytesFrom, 0, bytesFrom.Length);
                    string dataFromServer = Encoding.ASCII.GetString(bytesFrom);
                    dataFromServer = dataFromServer.Split('\0')[0];

                    Console.WriteLine("Server response: " + dataFromServer);
                    AddLineToSpdOutput(dataFromServer);

                    return dataFromServer.Length > 0 ? 0 : 1;
                }
                else
                {
                    AddLineToSpdOutput("ISKLJUČENO");
                    return 0;
                }
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
                RecieveMessages();
                if (measureSpeed)
                {
                    RecieveSpeed();
                }
                else
                {
                    AddLineToSpdOutput("ISKLJUČENO");
                }
                await Task.Delay(100);
            }

        }
        #endregion

        #region INTERACTIONS
        private void btn_quit_Click(object sender, RoutedEventArgs e)
        {
            CloseApp();
        }
        private async Task CloseApp()
        {
            await DisconnectFromServer();
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

        private void AddLineToSpdOutput(string line = "")
        {
            spdOutput = line;
            RefreshSpdOutput();
        }
        private void RefreshSpdOutput()
        {
            speedOutput.Text = "Brzina motora (RPM):\n" + spdOutput;
        }

        private void btn_measureSpeed_Click(object sender, RoutedEventArgs e)
        {
            ToggleSpeedMeasure();
            if (measureSpeed)
                SendCommand("TSI");
            else
                SendCommand("TSO");
        }
        private void ToggleSpeedMeasure()
        {
            measureSpeed = !measureSpeed;
            lb_measureSpeed.Content = measureSpeed ? "Uključeno" : "Isključeno";
            lb_measureSpeed.Foreground = measureSpeed ? greenColor : redColor;
            if (!measureSpeed) AddLineToSpdOutput("ISKLJUČENO");
        }
        #endregion

        private async void Window_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            quitComplete = false;
            await DisconnectFromServer();

            while(quitComplete) { await Task.Delay(10); }
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

        private void btn_setAccel_Click(object sender, RoutedEventArgs e)
        {
            string cmd = "ACC";
            float acc = 0;
            if (float.TryParse(tb_acceleration.Text, out acc))
            {
                if (acc < 0) acc = 0;
                if (acc > 10) acc = 10;
                tb_acceleration.Text = acc.ToString();
                SendCommand($"{cmd} {acc} 0");
            }
            else
            {
                MessageBox.Show("Uneta vrednost za ubrzanje nije dozvoljena.");
            }
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
