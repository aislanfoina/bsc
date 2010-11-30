#ifndef SPLOTCH_CUDA2_H
#define SPLOTCH_CUDA2_H

#include "cuda/splotch_cuda.h"

//things for combination with host threads
struct  param_combine_thread //for host combine thread
  {
  // bool    bFinished; used in thread combine. not working.
  bool a_eq_e;
  void *fbuf;
  int combineStartP, combineEndP;
  cu_particle_splotch *ps;
  float timeUsed;
  arr2<COLOUR> *pPic;
  };

#ifndef NO_WIN_THREAD
THREADFUNC combine(void *param);
#endif
THREADFUNC cu_thread_func(void *pinfo);
THREADFUNC cu_draw_chunk(void *pinfo);
THREADFUNC host_thread_func(void *pinfo);

//for record times
enum TimeRecords {
  CUDA_INIT,
  COPY2C_LIKE,
  RANGE,
  TRANSFORMATION,
  COLORIZE,
  FILTER,
  SORT,
  RENDER,
  COMBINE,
  THIS_THREAD,
  TIME_RECORDS   //to indicate number of times
  };

// struct containing thread task info
struct thread_info
  {
  int devID;                  //index of the device selected
  int startP, endP;           //start and end particles to handle
  arr2<COLOUR> *pPic;         //the output image
  float times[TIME_RECORDS];  //carry out times of computing
  };

//some global info shared by all threads
extern paramfile       *g_params;
extern std::vector<particle_sim> particle_data; //raw data from file
extern vec3 campos, lookat, sky;
extern std::vector<COLOURMAP> amap,emap;
extern int ptypes;

void DevideThreadsTasks(thread_info *tInfo, int nThread, bool bHostThread);

void render_cuda(paramfile &params, int &res, arr2<COLOUR> &pic);

//void cu_range_task(paramfile &params, cu_particle_sim* h_pd, unsigned int n, cu_gpu_vars* pgv);
void cu_range_task(paramfile &params, cu_particle_sim* h_pd, unsigned int n, cu_gpu_vars* pgv);
void cu_transform_task(paramfile &fparams, unsigned int n, double c[3], double l[3], double s[3], cu_particle_sim* h_pd, cu_gpu_vars* pgv);
void cu_colorize_task(paramfile &params, cu_particle_splotch *h_ps, int n, cu_gpu_vars* pgv);
void cu_render1_task(int startP, int endP, bool a_eq_e, double grayabsorb, cu_gpu_vars* pgv);

#endif
