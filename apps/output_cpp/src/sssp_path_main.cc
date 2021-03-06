#include "common_main.h"
#include "sssp_path.h"
#include "gm_rand.h"

class my_main: public main_t
{
private:
    int* dist;
    int* len;
    node_t* prev;
    gm_node_seq Q;
    node_t begin;
    node_t end;

public:
    virtual ~my_main() {
        delete[] dist;
        delete[] len;
        delete[] prev;
    }

    virtual bool prepare() {
        dist = new int[G.num_nodes()];
        len = new int[G.num_edges()];
        prev = new node_t[G.num_nodes()];

        // for NUMA, let each thread touch it first
        #pragma omp parallel for
        for (node_t i = 0; i < G.num_nodes(); i++)
            for (edge_t j = G.begin[i]; j < G.begin[i+1]; j++)
                len[j] = 0;

	    gm_rand32 xorshift_rng;
        for (edge_t i = 0; i < G.num_edges(); i++)
            len[i] = (xorshift_rng.rand() % 100) + 1;  // length: 1 ~ 100
        return true;
    }

    virtual bool run() {
        node_t root = rand() % G.num_nodes();
        begin = root;
        end = rand() % G.num_nodes();
        // compute all shortest paths from root
        sssp_path(G, dist, len, root, prev);
        // get specific instance from root to end
        get_path(G, begin, end, prev, Q);
        return true;
    }

    virtual bool post_process() {
        printf("shortest path from %d to %d\n", begin, end);
        gm_node_seq::seq_iter n_I = Q.prepare_seq_iteration();
        while (n_I.has_next())
        {
            node_t n = n_I.get_next();
            printf("%d", n);
            if(n_I.has_next())
                printf(" -> ");
            else
                printf("\n");
        }
        return true;
    }
};

int main(int argc, char** argv) {
    my_main M;
    M.main(argc, argv);
}
