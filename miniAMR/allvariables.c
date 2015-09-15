#include "mpi.h"

typedef struct {
   int number;
   int level;
   int refine;
   int new_proc;
   int parent;       // if original block -1,
                     // else if on node, number in structure
                     // else (-2 - parent->number)
   int parent_node;
   int child_number;
   int nei_refine[6];
   int nei_level[6];  /* 0 to 5 = W, E, S, N, D, U; use -2 for boundary */
   int nei[6][2][2];  /* negative if off processor (-1 - proc) */
   int cen[3];
   double ****array;
} block;
THREAD_VAR block *blocks;

typedef struct {
   int number;
   int level;
   int parent;      // -1 if original block
   int parent_node;
   int child_number;
   int refine;
   int child[8];    // n if on node, number if not
                    // if negative, then onnode child is a parent (-1 - n)
   int child_node[8];
   int cen[3];
} parent;
THREAD_VAR parent *parents;

typedef struct {
   int number;     // number of block
   int n;          // position in block array
} sorted_block;
THREAD_VAR sorted_block *sorted_list;
THREAD_VAR int *sorted_index;

THREAD_VAR int my_pe;
THREAD_VAR int num_pes;

THREAD_VAR int max_num_blocks;
THREAD_VAR int target_active;
THREAD_VAR int target_max;
THREAD_VAR int target_min;
THREAD_VAR int num_refine;
THREAD_VAR int uniform_refine;
THREAD_VAR int x_block_size, y_block_size, z_block_size;
THREAD_VAR int num_vars;
THREAD_VAR int comm_vars;
THREAD_VAR int init_block_x, init_block_y, init_block_z;
THREAD_VAR int reorder;
THREAD_VAR int npx, npy, npz;
THREAD_VAR int inbalance;
THREAD_VAR int refine_freq;
THREAD_VAR int report_diffusion;
THREAD_VAR int checksum_freq;
THREAD_VAR int stages_per_ts;
THREAD_VAR int error_tol;
THREAD_VAR int num_tsteps;
THREAD_VAR int stencil;
THREAD_VAR int report_perf;
THREAD_VAR int plot_freq;
THREAD_VAR int lb_opt;
THREAD_VAR int block_change;
THREAD_VAR int code;
THREAD_VAR int permute;
THREAD_VAR int nonblocking;
THREAD_VAR int refine_ghost;

THREAD_VAR int max_num_parents;
THREAD_VAR int num_parents;
THREAD_VAR int max_active_parent;
THREAD_VAR int cur_max_level;
THREAD_VAR int *num_blocks;
THREAD_VAR int *local_num_blocks;
THREAD_VAR int *block_start;
THREAD_VAR int num_active;
THREAD_VAR int max_active_block;
THREAD_VAR int global_active;
THREAD_VAR int x_block_half, y_block_half, z_block_half;
THREAD_VAR double tol;
THREAD_VAR double *grid_sum;
THREAD_VAR int *p8, *p2;
THREAD_VAR int mesh_size[3];
THREAD_VAR int max_mesh_size;
THREAD_VAR int *from, *to;
THREAD_VAR int msg_len[3][4];
THREAD_VAR int local_max_b;
THREAD_VAR int global_max_b;
THREAD_VAR int num_objects;
typedef struct {
   int type;
   int bounce;
   double cen[3];
   double orig_cen[3];
   double move[3];
   double orig_move[3];
   double size[3];
   double orig_size[3];
   double inc[3];
} object;
THREAD_VAR object *objects;

THREAD_VAR int num_dots;
THREAD_VAR int max_num_dots;
THREAD_VAR int max_active_dot;
typedef struct {
   int cen[3];
   int number;
   int n;
   int proc;
   int new_proc;
} dot;
THREAD_VAR dot *dots;

THREAD_VAR double average[128];
THREAD_VAR double stddev[128];
THREAD_VAR double minimum[128];
THREAD_VAR double maximum[128];

THREAD_VAR double timer_all;

THREAD_VAR double timer_comm_all;
THREAD_VAR double timer_comm_dir[3];
THREAD_VAR double timer_comm_recv[3];
THREAD_VAR double timer_comm_pack[3];
THREAD_VAR double timer_comm_send[3];
THREAD_VAR double timer_comm_same[3];
THREAD_VAR double timer_comm_diff[3];
THREAD_VAR double timer_comm_bc[3];
THREAD_VAR double timer_comm_wait[3];
THREAD_VAR double timer_comm_unpack[3];

THREAD_VAR double timer_calc_all;

THREAD_VAR double timer_cs_all;
THREAD_VAR double timer_cs_red;
THREAD_VAR double timer_cs_calc;

THREAD_VAR double timer_refine_all;
THREAD_VAR double timer_refine_co;
THREAD_VAR double timer_refine_mr;
THREAD_VAR double timer_refine_cc;
THREAD_VAR double timer_refine_sb;
THREAD_VAR double timer_refine_c1;
THREAD_VAR double timer_refine_c2;
THREAD_VAR double timer_refine_sy;
THREAD_VAR double timer_cb_all;
THREAD_VAR double timer_cb_cb;
THREAD_VAR double timer_cb_pa;
THREAD_VAR double timer_cb_mv;
THREAD_VAR double timer_cb_un;
THREAD_VAR double timer_target_all;
THREAD_VAR double timer_target_rb;
THREAD_VAR double timer_target_dc;
THREAD_VAR double timer_target_pa;
THREAD_VAR double timer_target_mv;
THREAD_VAR double timer_target_un;
THREAD_VAR double timer_target_cb;
THREAD_VAR double timer_target_ab;
THREAD_VAR double timer_target_da;
THREAD_VAR double timer_target_sb;
THREAD_VAR double timer_lb_all;
THREAD_VAR double timer_lb_sort;
THREAD_VAR double timer_lb_pa;
THREAD_VAR double timer_lb_mv;
THREAD_VAR double timer_lb_un;
THREAD_VAR double timer_lb_misc;
THREAD_VAR double timer_lb_mb;
THREAD_VAR double timer_lb_ma;
THREAD_VAR double timer_rs_all;
THREAD_VAR double timer_rs_ca;
THREAD_VAR double timer_rs_pa;
THREAD_VAR double timer_rs_mv;
THREAD_VAR double timer_rs_un;

THREAD_VAR double timer_plot;

THREAD_VAR long total_blocks;
THREAD_VAR int nb_min;
THREAD_VAR int nb_max;
THREAD_VAR int nrrs;
THREAD_VAR int nrs;
THREAD_VAR int nps;
THREAD_VAR int nlbs;
THREAD_VAR int num_refined;
THREAD_VAR int num_reformed;
THREAD_VAR int num_moved_all;
THREAD_VAR int num_moved_lb;
THREAD_VAR int num_moved_rs;
THREAD_VAR int num_moved_reduce;
THREAD_VAR int num_moved_coarsen;
THREAD_VAR int counter_halo_recv[3];
THREAD_VAR int counter_halo_send[3];
THREAD_VAR double size_mesg_recv[3];
THREAD_VAR double size_mesg_send[3];
THREAD_VAR int counter_face_recv[3];
THREAD_VAR int counter_face_send[3];
THREAD_VAR int counter_bc[3];
THREAD_VAR int counter_same[3];
THREAD_VAR int counter_diff[3];
THREAD_VAR int counter_malloc;
THREAD_VAR double size_malloc;
THREAD_VAR int counter_malloc_init;
THREAD_VAR double size_malloc_init;
THREAD_VAR int total_red;

THREAD_VAR double *send_buff, *recv_buff;    /* use in comm and for balancing blocks */

THREAD_VAR int s_buf_size, r_buf_size;

THREAD_VAR int num_comm_partners[3],  // number of comm partners in each direction
    *comm_partner[3],      // list of comm partners in each direction
    max_comm_part[3],      // lengths of comm partners arrays
    *send_size[3],         // send sizes for each comm partner
    *recv_size[3],         // recv sizes for each comm partner
    *comm_index[3],        // index into comm_block, _face_case, and offsets
    *comm_num[3],          // number of blocks for each comm partner
    *comm_block[3],        // array containing local block number for comm
    *comm_face_case[3],    // array containing face cases for comm
    *comm_pos[3],          // position for center of sending face
    *comm_pos1[3],         // perpendicular position of sending face
    *comm_send_off[3],     // offset into send buffer (global, convert to local)
    *comm_recv_off[3],     // offset into recv buffer
    num_cases[3],          // amount used in above six arrays
    max_num_cases[3],      // length of above six arrays
    s_buf_num[3],          // total amount being sent in each direction
    r_buf_num[3];          // total amount being received in each direction

THREAD_VAR MPI_Request *request, *s_req;

THREAD_VAR int max_num_req;

// for comm_parent - this is a non-symetric communication

typedef struct {
   int num_comm_part;          // number of other cores to communicate with
   int *comm_part;             // core to communicate with
   int *comm_num;              // number to communicate to each core
   int *index;                 // offset into next two arrays
   int *comm_b;                // block number to communicate from
   int *comm_p;                // parent number of block (for sorting)
   int *comm_c;                // child number of block
   int max_part;               // max communication partners
   int num_cases;              // number to communicate
   int max_cases;              // max number to communicate
} par_comm;
THREAD_VAR par_comm par_b, par_p, par_p1;

THREAD_VAR int *bin,
    *gbin;

THREAD_VAR MPI_Comm *comms;
THREAD_VAR int *me;
THREAD_VAR int *np;

