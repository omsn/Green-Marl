#include <stack>
#include <omp.h>
#include "gm.h"
#include "sssp_dijkstra.h"

/*
  void printPath(node_t startNodeKey,
                 node_t endNodeKey,
                 std::stack<edge_t>& pathEdges,
                 std::stack<node_t>& pathNodes,
                 double totalCost,
                 gm_graph& G,
                 int cutoff,
                 long* network_edge_keys) {
    node_t startNodeId = G.nodekey_to_nodeid(startNodeKey);
    node_t endNodeId = G.nodekey_to_nodeid(endNodeKey);
    printf("%d -> %d\n", startNodeKey, endNodeKey);
    printf("    Costs are %lf\n", totalCost);
    printf("    Number of links is %d\n", pathEdges.size());
    node_t prev_n = startNodeId;
    while (true) {
      if (pathEdges.size() == 0 || --cutoff == 0) break;
      edge_t e = pathEdges.top();
      pathEdges.pop();
      node_t n = pathNodes.top();
      pathNodes.pop();
      assert(getDstNode(G, e) == n);
      printf("        %d: %d - %d\n", network_edge_keys[e], G.nodeid_to_nodekey(prev_n), G.nodeid_to_nodekey(n));
      prev_n = n;
    }
  }
  */


int main(int argc, char** argv) {

    if (argc < 5) {
        printf ("Usage: ./%s <input_file> <src_key> <dst_key> <dbg>\n", argv[0]);
        exit(1);
    }
    gm_rt_set_num_threads(1); // gm_runtime.h

    char *inputFile = argv[1];

    //------------------------------
    // Empty graph creation
    //------------------------------
    gm_graph G;

    std::vector<VALUE_TYPE> vprop_schema;
    // format for sample.adj
    vprop_schema.push_back(GMTYPE_DOUBLE);
    
    std::vector<VALUE_TYPE> eprop_schema;
    // format for sample.adj
    eprop_schema.push_back(GMTYPE_DOUBLE);
    eprop_schema.push_back(GMTYPE_LONG);


    std::vector<void*> vertex_props;
    std::vector<void*> edge_props;

    //------------------------------
    // Read adjacency list graph
    //------------------------------
    // File format:
    // 
    // src_node src_node_cost dst_node1_id edge1_cost edge1_key ... dst_nodeN_id edgeN_cost edge_N_id
    //
    //
    //    printf ("Loading graph from file \'%s\' in adjacency list format...\n", inputFile);

    struct timeval T3, T4;    
    gettimeofday(&T3, NULL);
    G.load_adjacency_list(inputFile, vprop_schema, eprop_schema, vertex_props, edge_props, " \t", false);
    gettimeofday(&T4, NULL);
    printf("MY DIJKSTRA CSR C - GRAPH LOADING TIME (ms): %lf\n", (T4.tv_sec - T3.tv_sec) * 1000 + (T4.tv_usec - T3.tv_usec) * 0.001);

    //------------------------------
    // Print graph details for manual verification 
    //------------------------------
    //    printf ("Number of nodes = %d\n", G.num_nodes());
    //    printf ("Number of edges = %d\n", G.num_edges());


    gm_rt_set_num_threads(1); // gm_runtime.h

    double* edge_costs = (double*)edge_props[0];
    long* network_edge_keys = (long*)edge_props[1];
    gm_node_seq Q;

    //    node_t startNodeKey = 199535084;
    //    node_t endNodeKey = 199926436;
    
    printf("num_nodes = %d\n", G.num_nodes());
    node_t* G_Parent = new node_t[G.num_nodes()];
    edge_t* G_ParentEdge = new edge_t[G.num_nodes()];


    node_t startNodeKey = atol(argv[2]);
    node_t endNodeKey = atol(argv[3]);
    int dbg = atol(argv[4]);
     
    node_t startNodeId = G.nodekey_to_nodeid(startNodeKey);
    node_t endNodeId = G.nodekey_to_nodeid(endNodeKey);

    //    printf("GRAPH LOADED\n");
    //    fflush(stdout);
    struct timeval T1, T2;    
    gettimeofday(&T1, NULL);
    // compute all shortest paths from root
    //    CALLGRIND_START_INSTRUMENTATION;
    dijkstra(G, edge_costs, startNodeId, endNodeId, G_Parent, G_ParentEdge);
    // get specific instance from root to end

    fflush(stdout);
    double totalCost = get_path(G, startNodeId, endNodeId, G_Parent, G_ParentEdge, edge_costs, Q);
    //   CALLGRIND_STOP_INSTRUMENTATION;
    gettimeofday(&T2, NULL);
    printf("MY DIJKSTRA CSR C - COMPUTATION RUNNING TIME (ms): %lf\n", (T2.tv_sec - T1.tv_sec) * 1000 + (T2.tv_usec - T1.tv_usec) * 0.001);
    printf("cost = %lf\n", totalCost);
        
    //if (dbg != 0)
      //printPath(startNodeKey, endNodeKey, pathEdges, pathNodes, totalCost, G, 21, network_edge_keys);


}
