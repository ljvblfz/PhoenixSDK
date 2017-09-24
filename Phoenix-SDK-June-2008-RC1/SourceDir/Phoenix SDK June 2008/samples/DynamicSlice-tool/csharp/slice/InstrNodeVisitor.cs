using System;
using System.Collections.Generic;
using System.Text;

namespace SliceAnalysis
{
    /// <summary>
    /// To be plugged into a Phx.Graphs.NodeWalker to gather instructions
    /// in a graph.  
    /// </summary>
    class InstrNodeVisitor : Phx.Graphs.NodeVisitor
    {
        // Graph node to instruction mapping.
        private Dictionary<Phx.Graphs.Node, Phx.IR.Instruction> _map;
        private SliceInfo _sliceInfo;

        /// <summary>
        /// Creates an InstrNodeVisitor object.
        /// </summary>
        /// <param name="map">
        ///     A Node-to-Instruction mapping for lookup during graph traversal.
        /// </param>
        /// <param name="sliceInfo">
        ///     SliceInfo object to store the gathered instructions in.
        /// </param>
        public InstrNodeVisitor(Dictionary<Phx.Graphs.Node, Phx.IR.Instruction> map,
                                SliceInfo sliceInfo)
        {
            _map = map;
            _sliceInfo = sliceInfo;
        }

        public override void PreVisit(Phx.Graphs.Node node)
        {
            _sliceInfo.Add(_map[node]);
        }
    }
}
