using System;
using System.Diagnostics;
using System.Linq;
using InjectorCore;

namespace Loader.Console
{
    class Program
    {
        static void Main(string[] args)
        {
            var Matches = Process.GetProcessesByName(args[0]); var Proc = Matches[0];
            System.Console.Out.WriteLine("Injecting Process: " + Proc.ProcessName);
            Stage1.Inject(Proc, String.Join("|", args.Skip(1)));
        }
    }
}
