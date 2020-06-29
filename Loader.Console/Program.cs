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
            var Matches = Process.GetProcessesByName(args[0]);
            System.Console.Out.WriteLine("Injecting Process: " + Matches[0].ProcessName);
            Stage1.Inject(Matches[0], String.Join("|", args.Skip(1)));
        }
    }
}
