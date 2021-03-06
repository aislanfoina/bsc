/*
Copyright things go here.
*/
/*
CuPolicy is the class that knows the overall state of cuda application.
All 'magic numbers' are out of this class.
*/
#ifndef CUPOLICY_H
#define CUPOLICY_H

#include "cxxsupport/paramfile.h"
#include "cuda/splotch_cuda.h"
#include <cuda.h>
#include <cuda_runtime.h>

//#include <cutil_inline.h>
#include "cuda/cutil_inline.h"

class CuPolicy
  {
  private:
    unsigned int m_blockSize, maxregion, fbsize;
    void GetDims1(unsigned int n, dim3 *dimGrid, dim3 *dimBlock);

  public:
    CuPolicy(paramfile &Param);

    int GetMaxRegion();
    int GetFBufSize();
    int GetSizeDPD(unsigned int n);
    void GetDimsRange(unsigned int n, dim3 *dimGrid, dim3 *dimBlock);
    void GetDimsColorize(unsigned int n, dim3 *dimGrid, dim3 *dimBlock);
    void GetDimsRender(unsigned int n, dim3 *dimGrid, dim3 *dimBlock);
    void GetDimsCombine(unsigned int minx, unsigned int miny,
      unsigned int maxx, unsigned int maxy,dim3 *dimGrid, dim3 *dimBlock);
    void GetDimsPostProcess(int xres, int yres,dim3 *dimGrid, dim3 *dimBlock);
    unsigned int GetFBufSize(bool a_eq_e);
  };

#endif //CUPOLICY_H
