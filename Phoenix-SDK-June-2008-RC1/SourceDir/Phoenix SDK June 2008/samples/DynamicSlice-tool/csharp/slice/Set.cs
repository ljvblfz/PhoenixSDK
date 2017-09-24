using System;
using System.Collections.Generic;
using System.Text;

namespace SliceAnalysis.Utilities
{
    class Set<T> : IEnumerable<T>
    {
        // Private

        private System.Collections.Generic.Dictionary<T, object> _set;

        public Set()
        {
            _set = new Dictionary<T, object>();
        }

        public void Add (T element)
        {
            _set[element] = null;
        }

        #region IEnumerable<T> Members

        IEnumerator<T> IEnumerable<T>.GetEnumerator()
        {
            return _set.Keys.GetEnumerator();
        }

        #endregion

        #region IEnumerable Members

        System.Collections.IEnumerator 
        System.Collections.IEnumerable.GetEnumerator()
        {
            return _set.Keys.GetEnumerator();
        }

        #endregion
    }
}
