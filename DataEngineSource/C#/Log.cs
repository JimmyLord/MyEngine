using System.Runtime.CompilerServices;

namespace MyEngine
{ 
    public class Log
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)] public extern static void Info(string str);
        [MethodImplAttribute(MethodImplOptions.InternalCall)] public extern static void Error(string str);
    }
}
