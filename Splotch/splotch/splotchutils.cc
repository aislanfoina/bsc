#include <fstream>
#include "splotch/splotchutils.h"
#include "cxxsupport/mpi_support.h"
#include "kernel/transform.h"
#include "cxxsupport/walltimer.h"

using namespace std;

struct particle_new
  {
  COLOUR a;
  float32 x, y, rmax, steepness;
  };
struct locinfo
  {
  uint16 minx, maxx, miny, maxy;
  };

void create_new_particles (const vector<particle_sim> &in, bool a_eq_e,
  float64 gray, vector<particle_new> &out, vector<locinfo> &loc, vector<COLOUR> &qvec)
  {
  const float64 powtmp = pow(pi,1./3.);
  const float64 sigma0=powtmp/sqrt(2*pi);
  const float64 bfak=1./(2*sqrt(pi)*powtmp);
  const int maxpix=65000;

  tsize nactive=0;
  for (tsize i=0; i<in.size(); ++i)
    if (in[i].active) ++nactive;

  out.reserve(nactive);
  loc.reserve(nactive);
  if (!a_eq_e) qvec.reserve(nactive);
  for (tsize i=0; i< in.size(); ++i)
    {
    if (in[i].active)
      {
      particle_new p;
      p.x = in[i].x;
      p.y = in[i].y;
      p.a = in[i].e;
      if (!a_eq_e)
        qvec.push_back (COLOUR (p.a.r/(p.a.r+gray),p.a.g/(p.a.g+gray),p.a.b/(p.a.b+gray)));

      p.a = p.a *(-0.5*bfak/in[i].ro);
      const float64 min_change=8e-5;
      p.steepness = -0.5/(in[i].r*in[i].r*sigma0*sigma0);
//      float64 amax=max(abs(p.a.r),max(abs(p.a.g),abs(p.a.b)));
//      const float64 min_change=8e-5;
//      float64 attenuation = min_change/amax;
      float64 attenuation = 3.7e-2; // equivalent to rfac=1.5
      p.rmax = sqrt(max(0.,log(attenuation)/p.steepness));
//p.rmax=3*in[i].r;
      out.push_back(p);

      locinfo l;
      l.minx = uint16(max(0,min(maxpix,int(p.x-p.rmax+1))));
      l.maxx = uint16(max(0,min(maxpix,int(p.x+p.rmax+1))));
      l.miny = uint16(max(0,min(maxpix,int(p.y-p.rmax+1))));
      l.maxy = uint16(max(0,min(maxpix,int(p.y+p.rmax+1))));
      loc.push_back(l);
      }
    }
  }

const int chunkdim=200;

void render_new (const vector<particle_sim> &pold, arr2<COLOUR> &pic,
  bool a_eq_e, double grayabsorb, bool nopostproc)
  {
  planck_assert(a_eq_e || (mpiMgr.num_ranks()==1),
    "MPI only supported for A==E so far");
  const tsize maxpix=65000;
  planck_assert ((pic.size1()<maxpix) && (pic.size2()<maxpix),
    "image dimensions too large");
  vector<particle_new> p;
  vector<locinfo> loc;
  vector<COLOUR> qvec;
  create_new_particles (pold, a_eq_e, grayabsorb, p, loc, qvec);

  exptable xexp(-20.);

  int xres = pic.size1(), yres=pic.size2();
  pic.fill(COLOUR(0,0,0));

  work_distributor wd (xres,yres,chunkdim,chunkdim);

//#pragma omp parallel
{
  int chunk;
//#pragma omp for schedule(dynamic,1)
  for (chunk=0; chunk<wd.nchunks(); ++chunk)
    {
    int x0, x1, y0, y1;
    wd.chunk_info(chunk,x0,x1,y0,y1);
    arr2<COLOUR8> lpic(x1-x0,y1-y0);
    arr<float64> pre1(yres);
    lpic.fill(COLOUR8(0,0,0));
    int x0s=x0, y0s=y0;
    x1-=x0; x0=0; y1-=y0; y0=0;

    for (tsize m=0; m<p.size(); ++m)
      {
      int minx=loc[m].minx-x0s;
      if (minx>=x1) continue;
      minx=max(minx,x0);
      int maxx=loc[m].maxx-x0s;
      if (maxx<=x0) continue;
      maxx=min(maxx,x1);
      if (minx>=maxx) continue;
      int miny=loc[m].miny-y0s;
      if (miny>=y1) continue;
      miny=max(miny,y0);
      int maxy=loc[m].maxy-y0s;
      if (maxy<=y0) continue;
      maxy=min(maxy,y1);
      if (miny>=maxy) continue;

      float64 posx=p[m].x, posy=p[m].y;
      posx-=x0s; posy-=y0s;
      float64 rmax= p[m].rmax;
      COLOUR8 a=p[m].a;

      float64 stp=p[m].steepness;
      float64 att_max=xexp(stp*rmax*rmax);

      for (int y=miny; y<maxy; ++y)
        pre1[y] = xexp(stp*(y-posy)*(y-posy));

      if (a_eq_e)
        {
        for (int x=minx; x<maxx; ++x)
          {
          double pre2 = xexp(stp*(x-posx)*(x-posx));
          for (int y=miny; y<maxy; ++y)
            {
            float64 att = pre1[y]*pre2;
            if (att>att_max)
              {
              lpic[x][y].r += att*a.r;
              lpic[x][y].g += att*a.g;
              lpic[x][y].b += att*a.b;
              }
            }
          }
        }
      else
        {
        COLOUR8 q=qvec[m];

        for (int x=minx; x<maxx; ++x)
          {
          double pre2 = xexp(stp*(x-posx)*(x-posx));
          for (int y=miny; y<maxy; ++y)
            {
            float64 att = pre1[y]*pre2;
            if (att>att_max)
              {
              lpic[x][y].r += xexp.expm1(att*a.r)*(lpic[x][y].r-q.r);
              lpic[x][y].g += xexp.expm1(att*a.g)*(lpic[x][y].g-q.g);
              lpic[x][y].b += xexp.expm1(att*a.b)*(lpic[x][y].b-q.b);
              }
            }
          }
        }
      }

    for(int ix=0;ix<x1;ix++)
      for(int iy=0;iy<y1;iy++)
        pic[ix+x0s][iy+y0s]=lpic[ix][iy];
    }
}

  mpiMgr.allreduceRaw
    (reinterpret_cast<float *>(&pic[0][0]),3*xres*yres,MPI_Manager::Sum);
  if (!nopostproc)
    if (mpiMgr.master() && a_eq_e)
      for (int ix=0;ix<xres;ix++)
        for (int iy=0;iy<yres;iy++)
          {
          pic[ix][iy].r=-xexp.expm1(pic[ix][iy].r);
          pic[ix][iy].g=-xexp.expm1(pic[ix][iy].g);
          pic[ix][iy].b=-xexp.expm1(pic[ix][iy].b);
          }
  }

void render_classic (const vector<particle_sim> &p, arr2<COLOUR> &pic,
  bool a_eq_e, double grayabsorb, bool nopostproc)
  {
  const float64 rfac=1.5;
  const float64 powtmp = pow(pi,1./3.);
  const float64 sigma0=powtmp/sqrt(2*pi);
  const float64 bfak=1./(2*sqrt(pi)*powtmp);
  exptable xexp(-20.);

  int xres = pic.size1(), yres=pic.size2();
  pic.fill(COLOUR(0,0,0));

  work_distributor wd (xres,yres,chunkdim,chunkdim);

//#pragma omp parallel
{
  int chunk;
//#pragma omp for schedule(dynamic,1)
  for (chunk=0; chunk<wd.nchunks(); ++chunk)
    {
    int x0, x1, y0, y1;
    wd.chunk_info(chunk,x0,x1,y0,y1);
    arr2<COLOUR8> lpic(x1-x0,y1-y0);
    arr<double> pre1(yres);
    lpic.fill(COLOUR8(0.,0.,0.));
    int x0s=x0, y0s=y0;
    x1-=x0; x0=0; y1-=y0; y0=0;

    for (unsigned int m=0; m<p.size(); ++m)
      if (p[m].active)
        {
        float64 r=p[m].r;
        float64 posx=p[m].x, posy=p[m].y;
        posx-=x0s; posy-=y0s;
        float64 rfacr=rfac*r;

        int minx=int(posx-rfacr+1);
        if (minx>=x1) continue;
        minx=max(minx,x0);
        int maxx=int(posx+rfacr+1);
        if (maxx<=x0) continue;
        maxx=min(maxx,x1);
        if (minx>=maxx) continue;
        int miny=int(posy-rfacr+1);
        if (miny>=y1) continue;
        miny=max(miny,y0);
        int maxy=int(posy+rfacr+1);
        if (maxy<=y0) continue;
        maxy=min(maxy,y1);
        if (miny>=maxy) continue;

        COLOUR8 a=p[m].e, q(0,0,0);
        if (!a_eq_e)
          {
          COLOUR8 e=p[m].e;
          q=COLOUR8(e.r/(a.r+grayabsorb),e.g/(a.g+grayabsorb),e.b/(a.b+grayabsorb));
          }

        float64 radsq = rfacr*rfacr;
        float64 prefac1 = -0.5/(r*r*sigma0*sigma0);
        float64 prefac2 = -0.5*bfak/p[m].ro;
        for (int y=miny; y<maxy; ++y)
          pre1[y]=prefac2*xexp(prefac1*(y-posy)*(y-posy));

        for (int x=minx; x<maxx; ++x)
          {
          float64 xsq=(x-posx)*(x-posx);
          double pre2 = xexp(prefac1*xsq);

          for (int y=miny; y<maxy; ++y)
            {
            float64 dsq = (y-posy)*(y-posy) + xsq;
            if (dsq<radsq)
              {
              float64 fac = pre1[y]*pre2;
              if (a_eq_e)
                {
                lpic[x][y].r += fac*a.r;
                lpic[x][y].g += fac*a.g;
                lpic[x][y].b += fac*a.b;
                }
              else
                {
                lpic[x][y].r += xexp.expm1(fac*a.r)*(lpic[x][y].r-q.r);
                lpic[x][y].g += xexp.expm1(fac*a.g)*(lpic[x][y].g-q.g);
                lpic[x][y].b += xexp.expm1(fac*a.b)*(lpic[x][y].b-q.b);
                } // if a_eq_e
              } // if dsq<radsq
            } // y
          } // x
        } // for particle[m]

    for (int ix=0;ix<x1;ix++)
      for (int iy=0;iy<y1;iy++)
        pic[ix+x0s][iy+y0s]=lpic[ix][iy];
    } // for this chunk
} // #pragma omp parallel

  //cout << "SON QUA 4 " << mpiMgr.rank() <<"\n";
  mpiMgr.allreduceRaw
    (reinterpret_cast<float *>(&pic[0][0]),3*xres*yres,MPI_Manager::Sum);
  //cout << "SON QUA 4.1 " << mpiMgr.rank() <<"\n";

  if (!nopostproc)
    if (mpiMgr.master() && a_eq_e)
      for (int ix=0;ix<xres;ix++)
        for (int iy=0;iy<yres;iy++)
          {
          pic[ix][iy].r=-xexp.expm1(pic[ix][iy].r);
          pic[ix][iy].g=-xexp.expm1(pic[ix][iy].g);
          pic[ix][iy].b=-xexp.expm1(pic[ix][iy].b);
          }
  //cout << "SON QUA 5 " << mpiMgr.rank() <<"\n";
  }

double my_asinh (double val)
  { return log(val+sqrt(1.+val*val)); }


void add_colorbar(paramfile &params, arr2<COLOUR> &pic, vector<COLOURMAP> &amap)
  {
  int xres = pic.size1(), yres=pic.size2();
  int offset=0;
  int ptypes = params.find<int>("ptypes",1);

  for(int itype=0;itype<ptypes;itype++)
    {
    if (params.find<bool>("color_is_vector"+dataToString(itype),false))
      cout << " adding no color bar for type " << itype
           << " as it is color vector ..." << endl;
    else
      {
      cout << " adding color bar for type " << itype << " ..." << endl;
      for (int x=0; x<xres; x++)
        {
        COLOUR e=amap[itype].getVal(x/float64(xres));
        for (int y=0; y<10; y++)
          pic[x][yres-offset-1-y] = e;
        }
      offset += 10;
      }
    }
  }


void particle_normalize(paramfile &params, vector<particle_sim> &p, bool verbose)
  {
  int ptypes = params.find<int>("ptypes",1);
  arr<bool> col_vector(ptypes),log_int(ptypes),log_col(ptypes),asinh_col(ptypes);
  arr<float32> mincol(ptypes,1e30),maxcol(ptypes,-1e30),
               minint(ptypes,1e30),maxint(ptypes,-1e30);

  for(int itype=0;itype<ptypes;itype++)
    {
    log_int[itype] = params.find<bool>("intensity_log"+dataToString(itype),true);
    log_col[itype] = params.find<bool>("color_log"+dataToString(itype),true);
    asinh_col[itype] = params.find<bool>("color_asinh"+dataToString(itype),false);
    col_vector[itype] = params.find<bool>("color_is_vector"+dataToString(itype),false);
    }

  int npart=p.size();

  for (int m=0; m<npart; ++m) // do log calculations if requested
    {
    if (log_int[p[m].type])
      p[m].I = log10(p[m].I);
    get_minmax(minint[p[m].type], maxint[p[m].type], p[m].I);

    if (log_col[p[m].type])
      p[m].C1 = log10(p[m].C1);
    if (asinh_col[p[m].type])
      p[m].C1 = my_asinh(p[m].C1);
    get_minmax(mincol[p[m].type], maxcol[p[m].type], p[m].C1);
    if (col_vector[p[m].type])
      {
      if (log_col[p[m].type])
        {
        p[m].C2 = log10(p[m].C2);
        p[m].C3 = log10(p[m].C3);
        }
      if (asinh_col[p[m].type])
        {
        p[m].C2 = my_asinh(p[m].C2);
        p[m].C3 = my_asinh(p[m].C3);
        }
      get_minmax(mincol[p[m].type], maxcol[p[m].type], p[m].C2);
      get_minmax(mincol[p[m].type], maxcol[p[m].type], p[m].C3);
      }
    }


  for(int itype=0;itype<ptypes;itype++)
    {
    mpiMgr.allreduce(minint[itype],MPI_Manager::Min);
    mpiMgr.allreduce(mincol[itype],MPI_Manager::Min);
    mpiMgr.allreduce(maxint[itype],MPI_Manager::Max);
    mpiMgr.allreduce(maxcol[itype],MPI_Manager::Max);

    float minval_int = params.find<float>("intensity_min"+dataToString(itype),minint[itype]);
    float maxval_int = params.find<float>("intensity_max"+dataToString(itype),maxint[itype]);
    float minval_col = params.find<float>("color_min"+dataToString(itype),mincol[itype]);
    float maxval_col = params.find<float>("color_max"+dataToString(itype),maxcol[itype]);

    if (verbose && mpiMgr.master())
      {
      cout << " For particles of type " << itype << " : " << endl;
      cout << " From data: " << endl;
      cout << " Color Range:     " << mincol[itype] << " (min) , " <<
                                      maxcol[itype] << " (max) " << endl;
      cout << " Intensity Range: " << minint[itype] << " (min) , " <<
                                      maxint[itype] << " (max) " << endl;
      cout << " Restricted to: " << endl;
      cout << " Color Range:     " << minval_col << " (min) , " <<
                                      maxval_col << " (max) " << endl;
      cout << " Intensity Range: " << minval_int << " (min) , " <<
                                      maxval_int << " (max) " << endl;
      }

    for(int m=0; m<npart; ++m)
      {
      if(p[m].type == itype)///clamp into (min,max)
        {
        my_normalize(minval_int,maxval_int,p[m].I);
        my_normalize(minval_col,maxval_col,p[m].C1);
        if (col_vector[p[m].type])
          {
          my_normalize(minval_col,maxval_col,p[m].C2);
          my_normalize(minval_col,maxval_col,p[m].C3);
          }
        }
      }
    }
  }


void particle_project(paramfile &params, vector<particle_sim> &p,
  const vec3 &campos, const vec3 &lookat, vec3 sky)
  {
  int res = params.find<int>("resolution",200);
  double fov = params.find<double>("fov",45); //in degrees
  double fovfct = tan(fov*0.5*degr2rad);
  int npart=p.size();

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

  float64 dist = (campos-lookat).Length();
  float64 xfac = 1./(fovfct*dist);
  if (!projection)
    cout << " Field of fiew: " << 1./xfac*2. << endl;

  bool minhsmlpixel = params.find<bool>("minhsmlpixel",false);

//#pragma omp parallel
{
  long m;
//#pragma omp for schedule(static)
  for (m=0; m<npart; ++m)
    {
    vec3 v(p[m].x,p[m].y,p[m].z);
    v=trans.TransPoint(v);
    p[m].x=v.x; p[m].y=v.y; p[m].z=v.z;

    if (!projection)
      {
      p[m].x = res*.5*(p[m].x+fovfct*dist)*xfac;
      p[m].y = res*.5*(p[m].y+fovfct*dist)*xfac;
      }
    else
      {
      xfac=1./(fovfct*p[m].z);
      p[m].x = res*.5*(p[m].x+fovfct*p[m].z)*xfac;
      p[m].y = res*.5*(p[m].y+fovfct*p[m].z)*xfac;
      }
    p[m].ro = p[m].r;
    p[m].r = p[m].r *res*.5*xfac;
    if (minhsmlpixel && (p[m].r>=0.0))
      {
      p[m].r = sqrt(p[m].r*p[m].r + .5*.5);
      p[m].ro = p[m].r/(res*.5*xfac);
      }
    }
}
  }

void particle_colorize(paramfile &params, vector<particle_sim> &p,
  vector<COLOURMAP> &amap, vector<COLOURMAP> &emap)
  {
  int res = params.find<int>("resolution",200);
  int ycut0 = params.find<int>("ycut0",0);
  int ycut1 = params.find<int>("ycut1",res);
  float zmaxval = params.find<float>("zmax",1.e23);
  float zminval = params.find<float>("zmin",0.0);
  int ptypes = params.find<int>("ptypes",1);
  arr<bool> col_vector(ptypes);
  arr<float64> brightness(ptypes),grayabsorb(ptypes);

  for(int itype=0;itype<ptypes;itype++)
    {
    brightness[itype] = params.find<double>("brightness"+dataToString(itype),1.);
    grayabsorb[itype] = params.find<float>("gray_absorption"+dataToString(itype),0.2);
    col_vector[itype] = params.find<bool>("color_is_vector"+dataToString(itype),false);
    }
  float64 rfac=1.5;
  int npart=p.size();

  for (int m=0; m<npart; ++m)
    {
    p[m].active = false;
    if (p[m].z<=0) continue;
    if (p[m].z<=zminval) continue;
    if (p[m].z>=zmaxval) continue;
    float64 r=p[m].r;
    float64 posx=p[m].x, posy=p[m].y;
    float64 rfacr=rfac*r;

    int minx=int(posx-rfacr+1);
    if (minx>=res) continue;
    minx=max(minx,0);
    int maxx=int(posx+rfacr+1);
    if (maxx<=0) continue;
    maxx=min(maxx,res);
    if (minx>=maxx) continue;
    int miny=int(posy-rfacr+1);
    if (miny>=ycut1) continue;
    miny=max(miny,ycut0);
    int maxy=int(posy+rfacr+1);
    if (maxy<=ycut0) continue;
    maxy=min(maxy,ycut1);
    if (miny>=maxy) continue;

    float64 col1=p[m].C1,col2=p[m].C2,col3=p[m].C3;
    clamp (0.0000001,0.9999999,col1);
    if (col_vector[p[m].type])
      {
      clamp (0.0000001,0.9999999,col2);
      clamp (0.0000001,0.9999999,col3);
      }
    float64 intensity=p[m].I;
    clamp (0.0000001,0.9999999,intensity);
    intensity *= brightness[p[m].type];
    COLOUR e;
    if (col_vector[p[m].type])
      {
      e.r=col1*intensity;
      e.g=col2*intensity;
      e.b=col3*intensity;
      }
    else
      e=amap[p[m].type].getVal(col1)*intensity;

    p[m].active = true;
    p[m].e = e;
    }
  }

void particle_sort(vector<particle_sim> &p, int sort_type, bool verbose)
  {
  switch(sort_type)
    {
    case 0:
      if (verbose && mpiMgr.master())
        cout << " skipped sorting ..." << endl;
      break;
    case 1:
      if (verbose && mpiMgr.master())
        cout << " sorting by z ..." << endl;
      sort(p.begin(), p.end(), zcmp());
      break;
    case 2:
      if (verbose && mpiMgr.master())
        cout << " sorting by value ..." << endl;
      sort(p.begin(), p.end(), vcmp1());
      break;
    case 3:
      if (verbose && mpiMgr.master())
        cout << " reverse sorting by value ..." << endl;
      sort(p.begin(), p.end(), vcmp2());
      break;
    case 4:
      if (verbose && mpiMgr.master())
        cout << " sorting by size ..." << endl;
      sort(p.begin(), p.end(), hcmp());
      break;
    default:
      planck_fail("unknown sorting chosen ...");
      break;
    }
  }

void timeReport()
  {
  if (mpiMgr.master())
    {
    wallTimers.stop("full");
    cout << endl << "--------------------------------------------" << endl;
    cout << "Summary of timings" << endl;
    cout << "Setup Data (secs)          : " << wallTimers.acc("setup") << endl;
    cout << "Read Data (secs)           : " << wallTimers.acc("read") << endl;
    cout << "Ranging Data (secs)        : " << wallTimers.acc("range") << endl;
    cout << "Transforming Data (secs)   : " << wallTimers.acc("transform") << endl;
    cout << "Sorting Data (secs)        : " << wallTimers.acc("sort") << endl;
    cout << "Coloring Sub-Data (secs)   : " << wallTimers.acc("coloring") << endl;
    cout << "Rendering Sub-Data (secs)  : " << wallTimers.acc("render") << endl;
    cout << "Write Data (secs)          : " << wallTimers.acc("write") << endl;
    cout << "Total (secs)               : " << wallTimers.acc("full") << endl;
    wallTimers.start("full");
    }
  }

void get_colourmaps (paramfile &params, vector<COLOURMAP> &amap,
  vector<COLOURMAP> &emap)
  {
  int ptypes = params.find<int>("ptypes",1);
  bool master = mpiMgr.master();
  amap.resize(ptypes);

  if (master)
    cout << "building color maps (" << ptypes << ")..." << endl;
  for (int itype=0;itype<ptypes;itype++)
    {
    if (params.find<bool>("color_is_vector"+dataToString(itype),false))
      {
      if (master)
        cout << " color of ptype " << itype << " is vector, so no colormap to load ..." << endl;
      }
    else
      {
      ifstream infile (params.find<string>("palette"+dataToString(itype)).c_str());
      planck_assert (infile,"could not open palette file  <" +
        params.find<string>("palette"+dataToString(itype)) + ">");
      string dummy;
      int nColours;
      infile >> dummy >> dummy >> nColours;
      if (master)
        cout << " loading " << nColours << " entries of color table of ptype " << itype << endl;
      double step = 1./(nColours-1);
      for (int i=0; i<nColours; i++)
        {
        float rrr,ggg,bbb;
        infile >> rrr >> ggg >> bbb;
        amap[itype].addVal(i*step,COLOUR(rrr/255,ggg/255,bbb/255));
        }
      }
    }
  emap=amap;
  }
