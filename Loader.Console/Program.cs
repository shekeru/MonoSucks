using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using InjectorCore;

namespace Loader.Console
{
    class Program
    {
        static void Main(string[] args)
        {
            AttachProc(args);
        }
        static void AttachProc(string[] args)
        {
            System.Console.Out.WriteLine("Process: " + args[0]);
            var Matches = Process.GetProcessesByName(args[0]);
            Injector Core = new Injector(Matches[0]);
            Core.Inject(args[1]);
        }
    }
}
