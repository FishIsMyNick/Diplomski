using System;
using System.Net;
using System.Net.Sockets;
using System.Runtime.CompilerServices;
using System.Text;

public class Server
{
    private static TcpListener serverSocket;
    private static NetworkStream networkStream;

    private static string serverIPv4 = "127.0.0.1";
    private static int serverPort = 12345;

    private static List<TcpClient> clientQueue = new List<TcpClient>();
    private static TcpClient client = null;
    private static List<Command> taskQueue = new List<Command>();

    private static bool running = true;
    private static bool listening = false;
    private static bool usingMotor = false;

    static async Task Main()
    {
        OpenServer();

        listening = true;
        ListenForClientLoop();
        ListenForCommandsLoop();
        ExecuteLoop();

        await Quit();
    }

    #region CONNECTION
    private static void OpenServer()
    {
        try
        {
            serverSocket = new TcpListener(IPAddress.Parse(serverIPv4), serverPort);
            serverSocket.Start();
            Console.WriteLine($"Server started on {serverIPv4}:{serverPort}");
        }
        catch (Exception e) { Console.WriteLine(e.Message); }
    }
    private static async Task ListenForConnections()
    {
        try
        {
            client = serverSocket.AcceptTcpClient();
            Console.WriteLine("Client connected");
        }
        catch (Exception e) { Console.WriteLine(e.Message); }
    }
    private static async Task<int> ListenForClientLoop()
    {
        while (listening)
        {
            if (client == null)
            {
                await ListenForConnections();
            }
            else
            {
                await Task.Delay(100);
            }
        }
        TcpClient clientSocket = null;

        {
            //while (running)
            //{
            //    while (clientQueue.Count == 0)
            //    {

            //        clientSocket = serverSocket.AcceptTcpClient();
            //        clientQueue.Add(clientSocket);
            //        Console.WriteLine("Client connected");
            //    }

            //    while (true)
            //    {
            //        networkStream = clientSocket.GetStream();
            //        try
            //        {

            //            byte[] bytesFrom = new byte[1024];
            //            networkStream.Read(bytesFrom, 0, bytesFrom.Length);

            //            string dataFromClient = Encoding.ASCII.GetString(bytesFrom);
            //            //dataFromClient = dataFromClient.Substring(0, dataFromClient.IndexOf("\0"));
            //            dataFromClient = dataFromClient.Split('\0')[0];

            //            Console.WriteLine("Received from client: " + dataFromClient);

            //            string serverResponse = "not available";
            //            byte[] sendBytes;
            //            Command cmd = ParseCommand(dataFromClient);


            //            switch (cmd)
            //            {
            //                case Command.None:
            //                    serverResponse = "No command sent.";
            //                    sendBytes = Encoding.ASCII.GetBytes(serverResponse);
            //                    networkStream.Write(sendBytes, 0, sendBytes.Length);
            //                    networkStream.Flush();
            //                    break;
            //                case Command.RotateCW:
            //                    serverResponse = "Starting command 1 -- <" + DateTime.Now + ">";
            //                    sendBytes = Encoding.ASCII.GetBytes(serverResponse);
            //                    networkStream.Write(sendBytes, 0, sendBytes.Length);
            //                    networkStream.Flush();

            //                    await MotorFunction1();

            //                    serverResponse = "Command 1 completed -- <" + DateTime.Now + ">";
            //                    sendBytes = Encoding.ASCII.GetBytes(serverResponse);
            //                    networkStream.Write(sendBytes, 0, sendBytes.Length);
            //                    networkStream.Flush();
            //                    break;
            //                case Command.RotateCCW:
            //                    serverResponse = "Starting command 2 -- <" + DateTime.Now + ">";
            //                    sendBytes = Encoding.ASCII.GetBytes(serverResponse);
            //                    networkStream.Write(sendBytes, 0, sendBytes.Length);
            //                    networkStream.Flush();

            //                    await MotorFunction2();

            //                    serverResponse = "Command 2 completed -- <" + DateTime.Now + ">";
            //                    sendBytes = Encoding.ASCII.GetBytes(serverResponse);
            //                    networkStream.Write(sendBytes, 0, sendBytes.Length);
            //                    networkStream.Flush();
            //                    break;
            //            }
            //        }
            //        catch (Exception ex)
            //        {
            //            Console.WriteLine("Client disconnected");
            //            clientQueue.Remove(clientSocket);
            //            client = null;
            //            break;
            //        }
            //    }
            //    clientSocket.Close();
            //}

            //serverSocket.Stop();
        }
        return 0;
    }
    private static async Task ListenForCommandsLoop()
    {
        while (client != null)
        {
            try
            {
                networkStream = client.GetStream();
                byte[] bytesFrom = new byte[1024];
                await networkStream.ReadAsync(bytesFrom, 0, bytesFrom.Length);

                string dataFromClient = Encoding.ASCII.GetString(bytesFrom);
                dataFromClient = dataFromClient.Split('\0')[0];

                Console.WriteLine("Received from client: " + dataFromClient);

                string serverResponse = "not available";
                byte[] sendBytes;
                taskQueue.Add(ParseCommand(dataFromClient));
            }
            catch (Exception ex)
            {
                client = null;
                await Console.Out.WriteLineAsync("Client lost connection unexpectedly.");
            }
            await Task.Delay(100);
        }
    }
    private static async Task ExecuteLoop()
    {
        string serverResponse = "not available";
        byte[] sendBytes;
        bool working = false;
        while (true)
        {
            while (taskQueue.Count > 0)
            {
                Command cmd = taskQueue[0];
                switch (cmd)
                {
                    case Command.None:
                        serverResponse = "No command sent.";
                        sendBytes = Encoding.ASCII.GetBytes(serverResponse);
                        await networkStream.WriteAsync(sendBytes, 0, sendBytes.Length);
                        networkStream.Flush();
                        break;
                    case Command.RotateCW:
                        serverResponse = "Starting command 1 -- <" + DateTime.Now + ">";
                        sendBytes = Encoding.ASCII.GetBytes(serverResponse);
                        await networkStream.WriteAsync(sendBytes, 0, sendBytes.Length);
                        networkStream.Flush();

                        await MotorFunction1();

                        serverResponse = "Command 1 completed -- <" + DateTime.Now + ">";
                        sendBytes = Encoding.ASCII.GetBytes(serverResponse);
                        await networkStream.WriteAsync(sendBytes, 0, sendBytes.Length);
                        networkStream.Flush();
                        break;
                    case Command.RotateCCW:
                        serverResponse = "Starting command 2 -- <" + DateTime.Now + ">";
                        sendBytes = Encoding.ASCII.GetBytes(serverResponse);
                        await networkStream.WriteAsync(sendBytes, 0, sendBytes.Length);
                        networkStream.Flush();

                        await MotorFunction2();

                        serverResponse = "Command 2 completed -- <" + DateTime.Now + ">";
                        sendBytes = Encoding.ASCII.GetBytes(serverResponse);
                        await networkStream.WriteAsync(sendBytes, 0, sendBytes.Length);
                        networkStream.Flush();
                        break;
                    case Command.Quit:
                        client = null;
                        await Console.Out.WriteLineAsync("Client disconnected.");
                        break;
                }
                taskQueue.RemoveAt(0);
            }
            await Task.Delay(100);
        }
    }
    private static async Task Quit()
    {
        while (true)
        {
            Task.Delay(100000);
        }
    }
    #endregion
    #region PARSING
    private static Command ParseCommand(string raw)
    {
        switch (raw)
        {
            case "komanda 1":
                return Command.RotateCW;
            case "komanda 2":
                return Command.RotateCCW;
            case "quit":
                return Command.Quit;
            default: return Command.None;
        }
    }
    #endregion

    #region MOTOR FUNCTION CALLS
    private static async Task MotorFunction1()
    {
        await Console.Out.WriteLineAsync("Starting function 1 -- <" + DateTime.Now + ">");
        await Task.Delay(1000);
        await Console.Out.WriteLineAsync("Function 1 complete -- <" + DateTime.Now + ">");

    }
    private static async Task MotorFunction2()
    {
        await Console.Out.WriteLineAsync("Starting function 2 -- <" + DateTime.Now + ">");
        await Task.Delay(2000);
        await Console.Out.WriteLineAsync("Function 2 complete -- <" + DateTime.Now + ">");

    }
    #endregion
}

public enum Command
{
    None = 0, RotateCW = 1, RotateCCW = 2, Quit
}