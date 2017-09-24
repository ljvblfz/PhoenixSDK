using System;
using System.Collections.Generic;
using System.Text;

namespace SliceAnalysis.Utilities
{
    class Pair<T,U>
    {
        private T _first;
        private U _second;

        public T First { get { return _first; } set { _first = value; } }
        public U Second { get { return _second; } set { _second = value; } }

        public Pair(T first, U second)
        {
            _first = first;
            _second = second;
        }
    }
}
