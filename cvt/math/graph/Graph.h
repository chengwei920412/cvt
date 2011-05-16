#ifndef CVT_GRAPH_H
#define CVT_GRAPH_H

#include <cvt/math/graph/GraphNode.h>
#include <cvt/math/graph/GraphVisitor.h>
#include <cvt/util/Stack.h>
#include <vector>
#include <queue>
#include <utility>

namespace cvt {

	/* Graph */
	template<typename TNODE, typename TEDGE>
	class Graph {
		public:
			typedef GraphEdge<TNODE,TEDGE> EDGETYPE;
			typedef GraphNode<TNODE,TEDGE> NODETYPE;

			Graph();
			~Graph();

			GraphNode<TNODE,TEDGE>* addNode( TNODE data );
			size_t nodeSize() const;
// FIXME:			size_t edgeSize() const;

			void dfs( GraphVisitor<TNODE,TEDGE>& visitor );
			void bfs( GraphVisitor<TNODE,TEDGE>& visitor );
			void topologicalSort( GraphTSVisitor<TNODE,TEDGE>& visitor );

		private:
			std::vector< GraphNode<TNODE,TEDGE>* > _nodes;
	};

	template<typename TNODE,typename TEDGE>
	inline Graph<TNODE,TEDGE>::Graph()
	{
	}

	template<typename TNODE,typename TEDGE>
	inline Graph<TNODE,TEDGE>::~Graph()
	{
		for( typename std::vector< GraphNode<TNODE,TEDGE>* >::const_iterator it = _nodes.begin(), end = _nodes.end(); it != end; ++it )
			delete *it;
		_nodes.clear();
	}

	template<typename TNODE,typename TEDGE>
	inline GraphNode<TNODE,TEDGE>* Graph<TNODE,TEDGE>::addNode( TNODE data )
	{
		GraphNode<TNODE,TEDGE>* node = new GraphNode<TNODE,TEDGE>( data );
		_nodes.push_back( node );
		return node;
	}

	template<typename TNODE,typename TEDGE>
	inline size_t Graph<TNODE,TEDGE>::nodeSize() const
	{
		return _nodes.size();
	}


	template<typename TNODE,typename TEDGE>
	inline void Graph<TNODE,TEDGE>::dfs( GraphVisitor<TNODE,TEDGE>& visitor )
	{
		Stack<GraphNode<TNODE,TEDGE>*> stack;
		GraphNode<TNODE,TEDGE>* node;
		GraphNode<TNODE,TEDGE>* ntmp;

		visitor.init();
		for( size_t i = 0, iend = _nodes.size(); i < iend; i++ ) {
			node = _nodes[ i ];
			node->_visited = false;
			visitor.initNode( node );
			if( !node->inSize() )
				stack.push( node );
		}
		while( !stack.isEmpty() ) {
			node = stack.pop();
			for( size_t i = 0, end = node->outSize(); i < end; i++ ) {
				ntmp = node->outEdge( i )->dest();
				if( !ntmp->_visited )
					stack.push( ntmp );
			}
			node->_visited = true;
			visitor.visitNode( node );
		}
	}

	template<typename TNODE,typename TEDGE>
	inline void Graph<TNODE,TEDGE>::bfs( GraphVisitor<TNODE,TEDGE>& visitor )
	{
		std::queue<GraphNode<TNODE,TEDGE>*> queue;
		GraphNode<TNODE,TEDGE>* node;
		GraphNode<TNODE,TEDGE>* ntmp;

		visitor.init();
		for( size_t i = 0, iend = _nodes.size(); i < iend; i++ ) {
			node = _nodes[ i ];
			node->_visited = false;
			visitor.initNode( node );
			if( !node->inSize() )
				queue.push( node );
		}
		while( !queue.empty() ) {
			node = queue.front();
			queue.pop();
			for( size_t i = 0, end = node->outSize(); i < end; i++ ) {
				ntmp = node->outEdge( i )->dest();
				if( !ntmp->_visited )
					queue.push( ntmp );
			}
			node->_visited = true;
			visitor.visitNode( node );
		}

	}

	template<typename TNODE,typename TEDGE>
	inline void  Graph<TNODE,TEDGE>::topologicalSort( GraphTSVisitor<TNODE,TEDGE>& visitor )
	{
		Stack< std::pair< GraphNode<TNODE,TEDGE>*, GraphNode<TNODE,TEDGE>* > > stack;
		GraphNode<TNODE,TEDGE>* node;
		GraphNode<TNODE,TEDGE>* ntmp;
		bool mark;

		visitor.init();
		for( size_t i = 0, iend = _nodes.size(); i < iend; i++ ) {
			node = _nodes[ i ];
			node->_visited = false;
			visitor.initNode( node );
			if( !node->inSize() )
				stack.push(  std::make_pair< GraphNode<TNODE,TEDGE>*, GraphNode<TNODE,TEDGE>* >( node,NULL ) );
		}
		while( !stack.isEmpty() ) {
			std::pair<GraphNode<TNODE,TEDGE>*,GraphNode<TNODE,TEDGE>*> pair = stack.pop();
			node = pair.first;
			node->_visited = true;
			visitor.visitNode( node, pair.second );

			for( size_t i = 0, iend = node->outSize(); i < iend; i++ ) {
				mark = true;
				ntmp = node->outEdge( i )->dest();
				if( ntmp->_visited  )
					continue;
				for( size_t k = 0, kend = ntmp->inSize(); k < kend; k++ ) {
					if( ntmp->inEdge( k )->source() != node && !ntmp->inEdge( k )->source()->_visited ) {
						mark = false;
						break;
					}
				}
				if( mark )
					stack.push(  std::make_pair< GraphNode<TNODE,TEDGE>*, GraphNode<TNODE,TEDGE>* >( ntmp, node ) );
			}
		}
	}
}

#endif
