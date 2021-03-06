/*
Try accelerating splotch with CUDA. July 2009.
Copyright things go here.
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <cuda.h>
//#include <cutil_inline.h>
#include "cuda/cutil_inline.h"

#include "splotch/splotchutils.h"
#include "kernel/transform.h"

#include "cuda/splotch_kernel.cu"
#include "cuda/splotch_cuda.h"
#include "cuda/CuPolicy.h"


using namespace std;

template<typename T> T findParamWithoutChange
  (paramfile *param, std::string &key, T &deflt)
  {
  return param->param_present(key) ? param->find<T>(key) : deflt;
  }

#define CLEAR_MEM(p) if(p) {cutilSafeCall(cudaFree(p)); p=0;}
extern "C"
void getCuTransformParams(cu_param_transform &para_trans,
paramfile &params, double c[3], double l[3], double s[3])
  {
  vec3 campos(c[0], c[1], c[2]),
       lookat(l[0], l[1], l[2]),
       sky   (s[0], s[1], s[2]);

  int res = params.find<int>("resolution",200);
  double fov = params.find<double>("fov",45); //in degrees
  double fovfct = tan(fov*0.5*degr2rad);
  float64 xfac=0.0, dist=0.0;

  sky.Normalize();
  vec3 zaxis = (lookat-campos).Norm();
  vec3 xaxis = crossprod (sky,zaxis).Norm();
  vec3 yaxis = crossprod (zaxis,xaxis);
  TRANSFORM trans;
  trans.Make_General_Transform
        (TRANSMAT(xaxis.x,xaxis.y,xaxis.z,
                  yaxis.x,yaxis.y,yaxis.z,
                  zaxis.x,zaxis.y,zaxis.z,
                  0,0,0));
  trans.Invert();
  TRANSFORM trans2;
  trans2.Make_Translation_Transform(-campos);
  trans2.Add_Transform(trans);
  trans=trans2;
  bool projection = params.find<bool>("projection",true);

  if (!projection)
    {
    dist= (campos-lookat).Length();
    xfac=1./(fovfct*dist);
    cout << " Field of fiew: " << 1./xfac*2. << endl;
    }

  bool minhsmlpixel = params.find<bool>("minhsmlpixel",false);

  //retrieve the parameters for tansformation
  for (int i=0; i<12; i++)
    para_trans.p[i] =trans.Matrix().p[i];
  para_trans.projection=projection;
  para_trans.res=res;
  para_trans.fovfct=fovfct;
  para_trans.dist=dist;
  para_trans.xfac=xfac;
  para_trans.minhsmlpixel=minhsmlpixel;
  }

extern "C"
void cu_init(paramfile &params, int devID, cu_gpu_vars* pgv)
  {
//A  cudaSetDevice (devID); // initialize cuda runtime

//  int d;
//  cudaGetDevice(&d);
//  printf("\nDevice being used %d\n", d);

  pgv->policy =new CuPolicy(params); // Initialize pgv->policy class
  }

/**
 *
 * TODO: Have to populate the PGV variable and send it as paramemter to the task
 * k_range1<<<dimGrid,dimBlock>>>(d_pr, pgv->d_pd, n);
 *
 */

extern "C"
void cu_range(paramfile &params ,cu_particle_sim* h_pd,
  unsigned int n, cu_gpu_vars* pgv)
  {
  //allocate device memory for particle data
  int s =pgv->policy->GetSizeDPD(n); //allocate device memory for particle data

//// cu_gpu_vars* pgv populate solve this mallocs.

  //one more space allocated for the dumb

//A  cutilSafeCall(cudaMalloc((void**) &pgv->d_pd, s +sizeof(cu_particle_sim)));

  //copy particle data to device
//A  cutilSafeCall(cudaMemcpy(pgv->d_pd, h_pd, s, cudaMemcpyHostToDevice) );

  //ask for dims from pgv->policy
  dim3 dimGrid, dimBlock;
  pgv->policy->GetDimsRange(n, &dimGrid, &dimBlock);

  //prepare parameters for stage 1
  cu_param_range pr;
  int ptypes = params.find<int>("ptypes",1);
  pr.ptypes =ptypes;
  //now collect parameters from configuration
  for(int itype=0;itype<ptypes;itype++)
    {
    pr.log_int[itype] = params.find<bool>("intensity_log"+dataToString(itype),true);
    pr.log_col[itype] = params.find<bool>("color_log"+dataToString(itype),true);
    pr.asinh_col[itype] = params.find<bool>("color_asinh"+dataToString(itype),false);
    pr.col_vector[itype] = params.find<bool>("color_is_vector"+dataToString(itype),false);
    pr.mincol[itype]=1e30;
    pr.maxcol[itype]=-1e30;
    pr.minint[itype]=1e30;
    pr.maxint[itype]=-1e30;
    }
  //allocate memory on device and dump parameters to it
  cu_param_range  *d_pr=0;
  s =sizeof(cu_param_range);
  cutilSafeCall( cudaMalloc((void**) &d_pr, s) );
  cutilSafeCall(cudaMemcpy(d_pr, &pr, s, cudaMemcpyHostToDevice) );

  // call device for stage 1
  k_range1<<<dimGrid,dimBlock>>>(d_pr, pgv->d_pd, n);

  // copy out particles for min_maxes
  s =pgv->policy->GetSizeDPD(n);
  cutilSafeCall( cudaMemcpy( h_pd, pgv->d_pd, s, cudaMemcpyDeviceToHost) );

  //find the min-maxes
  for (int m=0; m<n; m++)
    {
    get_minmax(pr.minint[h_pd[m].type], pr.maxint[h_pd[m].type], h_pd[m].I);
    get_minmax(pr.mincol[h_pd[m].type], pr.maxcol[h_pd[m].type], h_pd[m].C1);
    if (pr.col_vector[h_pd[m].type])
      {
      get_minmax(pr.mincol[h_pd[m].type], pr.maxcol[h_pd[m].type], h_pd[m].C2);
      get_minmax(pr.mincol[h_pd[m].type], pr.maxcol[h_pd[m].type], h_pd[m].C3);
      }
    }

  // call device for stage 2 ptypes times
  // prepare parameters1 first
  float minval_int, maxval_int, minval_col, maxval_col;
  std::string tmp;
  for(int itype=0;itype<ptypes;itype++)
    {
    tmp = "intensity_min"+dataToString(itype);
    minval_int =findParamWithoutChange<float>(&params,  //in mid of developing only
      tmp, pr.minint[itype]);
    tmp = "intensity_max"+dataToString(itype);
    maxval_int = findParamWithoutChange<float>(&params, tmp, pr.maxint[itype]);
    tmp = "color_min"+dataToString(itype);
    minval_col = findParamWithoutChange<float>(&params, tmp, pr.mincol[itype]);
    tmp = "color_max"+dataToString(itype);
    maxval_col = findParamWithoutChange<float>(&params, tmp, pr.maxcol[itype]);

    k_range2<<<dimGrid, dimBlock>>>(d_pr, pgv->d_pd, n, itype,
      minval_int,maxval_int,minval_col,maxval_col);
    }

  //copy result out to host
  //in mid of development only!!!
  cutilSafeCall(cudaMemcpy(h_pd, pgv->d_pd, s, cudaMemcpyDeviceToHost) );

  //free parameters on device
  CLEAR_MEM((d_pr));

  //pgv->d_pd will be freed in cu_end
  }//cu range over

extern "C" void cu_transform (paramfile &fparams, unsigned int n,
  double c[3], double l[3], double s[3], cu_particle_sim* h_pd, cu_gpu_vars* pgv)
  {
  //retrieve pamaters for transformaiont first
  cu_param_transform tparams;
  getCuTransformParams(tparams,fparams,c,l,s);

  //arrange memory for the parameters and copy to device
  cu_param_transform  *d_pt;
  int size =sizeof(cu_param_transform);
  cutilSafeCall( cudaMalloc((void**) &d_pt, size) );
  cutilSafeCall(cudaMemcpy(d_pt, &tparams, size, cudaMemcpyHostToDevice) );

  //Get block dim and grid dim from pgv->policy object
  dim3 dimGrid, dimBlock;
  pgv->policy->GetDimsRange(n, &dimGrid, &dimBlock);

  //call device transformation
  k_transform<<<dimGrid,dimBlock>>>(pgv->d_pd, n, d_pt);

  //free parameters' device memory
  CLEAR_MEM((d_pt));

  //copy result out to host
  //in mid of development only!!!
  size =pgv->policy->GetSizeDPD(n);
  cutilSafeCall(cudaMemcpy(h_pd, pgv->d_pd, size, cudaMemcpyDeviceToHost) );
  }

extern "C" void cu_init_colormap(cu_colormap_info h_info, cu_gpu_vars* pgv)
  {
  //allocate memories for colormap and ptype_points and dump host data into it
  int size =sizeof(cu_color_map_entry) *h_info.mapSize;
  cutilSafeCall( cudaMalloc((void**) &pgv->d_colormap_info.map, size));
  cutilSafeCall(cudaMemcpy(pgv->d_colormap_info.map, h_info.map,
    size, cudaMemcpyHostToDevice) );
  //type
  size =sizeof(int) *h_info.ptypes;
  cutilSafeCall( cudaMalloc((void**) &pgv->d_colormap_info.ptype_points, size));
  cutilSafeCall(cudaMemcpy(pgv->d_colormap_info.ptype_points, h_info.ptype_points,
    size, cudaMemcpyHostToDevice) );

  //set fields of global varible pgv->d_colormap_info
  pgv->d_colormap_info.mapSize =h_info.mapSize;
  pgv->d_colormap_info.ptypes  =h_info.ptypes;
  }

extern "C" void cu_colorize(paramfile &params, cu_particle_splotch *h_ps,
  int n, cu_gpu_vars* pgv)
  {
  //fetch parameters for device calling first
  cu_param_colorize   pcolorize;
  pcolorize.res       = params.find<int>("resolution",200);
  pcolorize.ycut0     = params.find<int>("ycut0",0);
  pcolorize.ycut1     = params.find<int>("ycut1",pcolorize.res);
  pcolorize.zmaxval   = params.find<float>("zmax",1.e23);
  pcolorize.zminval   = params.find<float>("zmin",0.0);
  pcolorize.ptypes    = params.find<int>("ptypes",1);

  for(int itype=0; itype<pcolorize.ptypes; itype++)
    {
    pcolorize.brightness[itype] = params.find<double>("brightness"+dataToString(itype),1.);
    pcolorize.grayabsorb[itype] = params.find<float>("gray_absorption"+dataToString(itype),0.2);
    pcolorize.col_vector[itype] = params.find<bool>("color_is_vector"+dataToString(itype),false);
    }
  pcolorize.rfac=1.5;

  //prepare memory for parameters and dump to device
  cu_param_colorize   *d_param_colorize;
  cutilSafeCall( cudaMalloc((void**) &d_param_colorize, sizeof(cu_param_colorize)));
  cutilSafeCall( cudaMemcpy(d_param_colorize, &pcolorize, sizeof(cu_param_colorize),
    cudaMemcpyHostToDevice));

  //now prepare memory for d_particle_splotch.
  //one more for dums
  int size =n* sizeof(cu_particle_splotch);
  cutilSafeCall( cudaMalloc((void**) &pgv->d_ps_colorize, size+sizeof(cu_particle_splotch)));

  //fetch grid dim and block dim and call device
  dim3 dimGrid, dimBlock;
  pgv->policy->GetDimsColorize(n, &dimGrid, &dimBlock);
  k_colorize<<<dimGrid,dimBlock>>>(d_param_colorize, pgv->d_pd, n, pgv->d_ps_colorize,pgv->d_colormap_info);

  //copy the result out
  cutilSafeCall(cudaMemcpy(h_ps, pgv->d_ps_colorize, size, cudaMemcpyDeviceToHost) );

  //free params memory
  CLEAR_MEM((d_param_colorize));

  //device particle_sim memory can be freed now!
  CLEAR_MEM(pgv->d_pd);

  //particle_splotch memory on device will be freed in cu_end
  }

extern "C" int cu_get_max_region(cu_gpu_vars* pgv)
  {
  if (!pgv->policy) return -1;
  return pgv->policy->GetMaxRegion();
  }

extern "C" int cu_get_fbuf_size(cu_gpu_vars* pgv)
  {
  if (!pgv->policy) return -1;
  return pgv->policy->GetFBufSize();
  }

extern "C" void cu_init_exptab(double maxexp, cu_gpu_vars* pgv)
  {
  //set common fileds of pgv->d_exp_info
  pgv->d_exp_info.expfac =pgv->d_exp_info.dim2 / maxexp;
  //now make up tab1 and tab2 in host
  int dim1 =pgv->d_exp_info.dim1, dim2 =pgv->d_exp_info.dim2;
  float *h_tab1 =new float[dim1];
  float *h_tab2 =new float[dim2];
  for (int m=0; m<dim1; ++m)
    {
    h_tab1[m]=exp(m*dim1/pgv->d_exp_info.expfac);
    h_tab2[m]=exp(m/pgv->d_exp_info.expfac);
    }

  //allocate device memory and dump
  int size =sizeof(float) *dim1;
  cutilSafeCall( cudaMalloc((void**) &pgv->d_exp_info.tab1, size));
  cutilSafeCall( cudaMemcpy(pgv->d_exp_info.tab1, h_tab1, size,
    cudaMemcpyHostToDevice));
  size =sizeof(float) *dim2;
  cutilSafeCall( cudaMalloc((void**) &pgv->d_exp_info.tab2, size));
  cutilSafeCall( cudaMemcpy(pgv->d_exp_info.tab2, h_tab2, size,
    cudaMemcpyHostToDevice));

  //delete tab1 and tab2 in host
  delete []h_tab1;
  delete []h_tab2;
  }

extern "C" void cu_prepare_render(cu_particle_splotch *p,
  int n, cu_gpu_vars* pgv)
  {
  //init exp table
  cu_init_exptab(MAX_EXP, pgv);

  //allocate new memory as it may grow longer after spliting
  CLEAR_MEM(pgv->d_ps_colorize);
  int size = (n+1) *sizeof(cu_particle_splotch);
  cutilSafeCall( cudaMalloc((void**) &pgv->d_ps_render, size));

  //copy filtered particles into device
  size = n *sizeof(cu_particle_splotch);
  cutilSafeCall(cudaMemcpy(pgv->d_ps_render, p,size,
    cudaMemcpyHostToDevice) );

  //allocate fragment buffer memory on device
  size =cu_get_fbuf_size( pgv);
  cutilSafeCall( cudaMalloc((void**) &pgv->d_fbuf, size));
  }

extern "C" void cu_render1
  (int startP, int endP, bool a_eq_e, double grayabsorb, cu_gpu_vars* pgv)
  {
  //endP actually exceed the last one to render
  //get dims from pgv->policy object first
  dim3 dimGrid, dimBlock;
  pgv->policy->GetDimsRender(endP-startP, &dimGrid, &dimBlock);

  //call device
  k_render1<<<dimGrid, dimBlock>>>(pgv->d_ps_render, startP, endP,
    pgv->d_fbuf, a_eq_e, grayabsorb,pgv->d_exp_info);
  }

extern "C" void cu_get_fbuf
  (void *h_fbuf, bool a_eq_e, unsigned long n, cu_gpu_vars* pgv)
  {
  int size;
  if (a_eq_e)
    size =n* sizeof(cu_fragment_AeqE);
  else
    size =n* sizeof(cu_fragment_AneqE);

  cutilSafeCall(cudaMemcpy(h_fbuf, pgv->d_fbuf,size,
    cudaMemcpyDeviceToHost) );
  }

extern "C"
void    cu_end(cu_gpu_vars* pgv)
  {
  CLEAR_MEM((pgv->d_pd));
  CLEAR_MEM((pgv->d_ps_colorize));
  CLEAR_MEM((pgv->d_colormap_info.map));
  CLEAR_MEM((pgv->d_colormap_info.ptype_points));
  CLEAR_MEM((pgv->d_exp_info.tab1));
  CLEAR_MEM((pgv->d_exp_info.tab2));
  CLEAR_MEM((pgv->d_fbuf));
  CLEAR_MEM((pgv->d_pic));

  cudaThreadExit();

  delete pgv->policy;
  }

extern "C" int cu_get_chunk_particle_count(paramfile &params)
  {
  int M = 1<<20;
  int gMemSize =params.find<int>("graphics_memory_size", 256)*M;
  int fBufSize =params.find<int>("fragment_buffer_size", 128)*M;
  float factor =params.find<float>("particle_mem_factor", 4.0);
  int spareMem = 1*M;

  if (gMemSize <= fBufSize) return -1;

  return ( gMemSize -fBufSize -spareMem)/sizeof(cu_particle_sim) /factor;
  }
