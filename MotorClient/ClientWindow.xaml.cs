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
        private TcpClient clientSocket;
        private NetworkStream networkStream;

        private bool listening = false;
        private string cmdOutput;
        public ClientWindow()
        {
            cmdOutput = "";

            InitializeComponent();
            RefreshCmdOutput();

            string command = "L";

            int response = ConnectToServer();
            listening = true;
            ListenToServer();
        }

        #region CONNECTION
        //private static string serverIPv4 = "188.2.24.251";
        private static string serverIPv4 = "192.168.0.100";
        private int serverPort = 12345;

        private int ConnectToServer()
        {
            clientSocket = new TcpClient(serverIPv4, serverPort);
            networkStream = clientSocket.GetStream();
            return clientSocket == null ? 1 : 0;
        }
        private int DisconnectFromServer()
        {
            SendCommand("quit");
            clientSocket.Close();
            return 0;
        }
        private async Task<int> SendCommand(string command)
        {
            try
            {
                byte[] sendBytes = Encoding.ASCII.GetBytes(command);
                await networkStream.WriteAsync(sendBytes, 0, sendBytes.Length);
                networkStream.Flush();
                return 0;
            }
            catch (Exception ex)
            {
                await Console.Out.WriteLineAsync(ex.Message);
                return 1;
            }
        }
        private async Task<int> RecieveResponce()
        {
            try
            {
                byte[] bytesFrom = new byte[1024];
                await networkStream.ReadAsync(bytesFrom, 0, bytesFrom.Length);
                string dataFromServer = Encoding.ASCII.GetString(bytesFrom);
                dataFromServer = dataFromServer.Split('\0')[0];

                Console.WriteLine("Server response: " + dataFromServer);
                AddLineToCmdOutput("SERVER: " + dataFromServer);

                return dataFromServer.Length > 0 ? 0 : 1;
            }
            catch (Exception ex)
            {
                await Console.Out.WriteLineAsync(ex.Message);
                return 1;
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
            AddLineToCmdOutput("CLIENT: Sending rotate clockwise");
            string cmd = "RCW";
            float speed = 0;
            float.TryParse(tb_speed_cw.Text, out speed);
            float duration = 0;
            float.TryParse(tb_duration_cw.Text, out duration);
            SendCommand($"{cmd} {speed} {duration}");
        }
        private void btn_cmd2_Click(object sender, RoutedEventArgs e)
        {
            AddLineToCmdOutput("CLIENT: Sending rotate counter clockwise");
            string cmd = "RCCW";
            float speed = 0;
            float.TryParse(tb_speed_ccw.Text, out speed);
            float duration = 0;
            float.TryParse(tb_duration_ccw.Text, out duration);
            SendCommand($"{cmd} {speed} {duration}");
        }

        #endregion

        #region WINDOW
        private void AddLineToCmdOutput(string line = "")
        {
            cmdOutput += line + '\n';
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

        private void tb_speed_cw_TextChanged(object sender, TextChangedEventArgs e)
        {

        }
    }
}
