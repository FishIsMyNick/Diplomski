using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MotorServer
{
    public class CommandView
    {
        public string Komanda { get; set; }
        public string Brzina { get; set; }
        public string Trajanje { get; set; }
        public string TrajanjeVal { get => Trajanje.Split(' ')[0]; }
        public CommandView() { }
        public CommandView(string cmd, string speed, string duration)
        {
            Komanda = cmd;
            Brzina = speed;
            Trajanje = duration;
        }
    }
}
