/*
 *  This file is part of libcxxsupport.
 *
 *  libcxxsupport is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  libcxxsupport is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with libcxxsupport; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

/*
 *  libcxxsupport is being developed at the Max-Planck-Institut fuer Astrophysik
 *  and financially supported by the Deutsches Zentrum fuer Luft- und Raumfahrt
 *  (DLR).
 */

/*
 *  Copyright (C) 2009, 2010 Max-Planck-Society
 *  \author Martin Reinecke
 */

#ifndef PLANCK_MPI_SUPPORT_H
#define PLANCK_MPI_SUPPORT_H

#include "datatypes.h"
#include "arr.h"

class MPI_Manager
  {
  public:
    enum redOp { Sum, Min, Max };

  private:
    void gatherv_helper1_m (int nval_loc, arr<int> &nval, arr<int> &offset,
      int &nval_tot) const;
    void gatherRawVoid (const void *in, tsize num, void *out, NDT type) const;
    void gathervRawVoid (const void *in, tsize num, void *out,
      const int *nval, const int *offset, NDT type) const;

  public:
    MPI_Manager();
    ~MPI_Manager();

    int num_ranks() const;
    int rank() const;
    bool master() const;

    void barrier() const;

    void calcShare (int64 glo, int64 ghi, int64 &lo, int64 &hi) const
      { calcShareGeneral(glo,ghi,num_ranks(),rank(),lo,hi); }

    void sendRawVoid (const void *data, NDT type, tsize num, tsize dest) const;
    template<typename T> void sendRaw (const T *data, tsize num, tsize dest)
      const
      { sendRawVoid(data, nativeType<T>(), num, dest); }
    template<typename T> void send (const arr<T> &data, tsize dest) const
      { sendRaw(&data[0], data.size(), dest); }

    void recvRawVoid (void *data, NDT type, tsize num, tsize src) const;
    template<typename T> void recvRaw (T *data, tsize num, tsize src) const
      { recvRawVoid(data, nativeType<T>(), num, src); }
    template<typename T> void recv (arr<T> &data, tsize src) const
      { recvRaw(&data[0], data.size(), src); }

    void sendrecvRawVoid (const void *sendbuf, tsize sendcnt,
      tsize dest, void *recvbuf, tsize recvcnt, tsize src, NDT type) const;
    template<typename T> void sendrecvRaw (const T *sendbuf, tsize sendcnt,
      tsize dest, T *recvbuf, tsize recvcnt, tsize src) const
      {
      sendrecvRawVoid(sendbuf, sendcnt, dest, recvbuf, recvcnt, src,
                      nativeType<T>());
      }
    template<typename T> void sendrecv (const arr<T> &sendbuf, tsize dest,
      arr<T> &recvbuf, tsize src) const
      {
      sendrecvRaw(&sendbuf[0], sendbuf.size(), dest,
                  &recvbuf[0], recvbuf.size(), src);
      }

    void sendrecv_replaceRawVoid (void *data, NDT type, tsize num,
      tsize dest, tsize src) const;
    template<typename T> void sendrecv_replaceRaw (T *data, tsize num,
      tsize dest, tsize src) const
      { sendrecv_replaceRawVoid(data, nativeType<T>(), num, dest, src); }
    template<typename T> void sendrecv_replace (arr<T> &data, tsize dest,
      tsize src) const
      { sendrecv_replaceRaw(&data[0], data.size(), dest, src); }

    template<typename T> void gather_m (const T &in, arr<T> &out) const
      {
      out.alloc(num_ranks());
      gatherRawVoid (&in,1,&out[0],nativeType<T>());
      }
    template<typename T> void gather_s (const T &in) const
      { gatherRawVoid (&in,1,0,nativeType<T>()); }

    template<typename T> void gatherv_m (const arr<T> &in, arr<T> &out) const
      {
      int nval_loc = in.size(), nval_tot;
      arr<int> nval, offset;
      gatherv_helper1_m (nval_loc,nval,offset,nval_tot);
      out.alloc(nval_tot);
      gathervRawVoid (&in[0],nval_loc,&out[0],&nval[0],&offset[0],nativeType<T>());
      }
    template<typename T> void gatherv_s (const arr<T> &in) const
      {
      int nval_loc = in.size();
      gather_s (nval_loc);
      gathervRawVoid (&in[0],nval_loc,0,0,0,nativeType<T>());
      }

    template<typename T> void gatherv (const arr<T> &in, arr<T> &out) const
      { master() ? gatherv_m(in,out) : gatherv_s(in); }

    template<typename T> void gatherv_m (const arr2<T> &in, arr2<T> &out) const
      {
      int nval_loc = in.size(), nval_tot;
      arr<int> nval, offset;
      gatherv_helper1_m (nval_loc, nval, offset, nval_tot);
      out.alloc(nval_tot/in.size2(),in.size2());
      gathervRawVoid (&in[0][0],nval_loc,&out[0][0],&nval[0],&offset[0],nativeType<T>());
      }

    template<typename T> void gatherv_s (const arr2<T> &in) const
      {
      int nval_loc = in.size();
      gather_s (nval_loc);
      gathervRawVoid (&in[0][0],nval_loc,0,0,0,nativeType<T>());
      }

    void reduceRawVoid (const void *in, void *out, NDT type, tsize num,
      redOp op, int root) const;
    template<typename T> void reduceRaw (const T *in, T *out, tsize num,
      redOp op, int root) const
      { reduceRawVoid (in, out, nativeType<T>(), num, op, root); }
    template<typename T> void reduce (const arr<T> &in, arr<T> &out, redOp op,
      int root=0) const
      { (rank()==root) ? reduce_m (in, out, op) : reduce_s (in, op, root); }
    template<typename T> void reduce_m (const arr<T> &in, arr<T> &out,
      redOp op) const
      {
      out.alloc(in.size());
      reduceRaw (&in[0], &out[0], in.size(), op, rank());
      }
    template<typename T> void reduce_s (const arr<T> &in, redOp op, int root)
      const
      { reduceRaw (&in[0], 0, in.size(), op, root); }

    void allgatherRawVoid (const void *in, void *out, NDT type, tsize num)
      const;
    template<typename T> void allgatherRaw (const T *in, T *out, tsize num)
      const
      { allgatherRawVoid (in, out, nativeType<T>(), num); }
    template<typename T> void allgather (const arr<T> &in, arr<T> &out) const
      {
      out.alloc(num_ranks()*in.size());
      allgatherRaw (&in[0], &out[0], in.size());
      }
    template<typename T> void allgather (const T &in, arr<T> &out) const
      {
      out.alloc(num_ranks());
      allgatherRaw (&in, &out[0], 1);
      }

    void allreduceRawVoid (const void *in, void *out, NDT type, tsize num,
      redOp op) const;
    template<typename T> void allreduceRaw (const T *in, T *out, tsize num,
      redOp op) const
      { allreduceRawVoid (in, out, nativeType<T>(), num, op); }
    template<typename T> void allreduce (const arr<T> &in, arr<T> &out,
      redOp op) const
      {
      out.alloc(in.size());
      allreduceRaw (&in[0], &out[0], in.size(), op);
      }
    template<typename T> void allreduce (const T &in, T &out, redOp op) const
      { allreduceRaw (&in, &out, 1, op); }

    void allreduceRawVoid (void *data, NDT type, tsize num, redOp op) const;
    template<typename T> void allreduceRaw (T *data, tsize num,
      redOp op) const
      { allreduceRawVoid (data, nativeType<T>(), num, op); }
    template<typename T> void allreduce (arr<T> &data, redOp op) const
      { allreduceRaw (&data[0], data.size(), op); }
    template<typename T> void allreduce (T &data, redOp op) const
      { allreduceRaw (&data, 1, op); }

    void bcastRawVoid (void *data, NDT type, tsize num, int root) const;
    template<typename T> void bcastRaw (T *data, tsize num, int root) const
      { bcastRawVoid (data, nativeType<T>(), num, root); }
    template<typename T> void bcast (arr<T> &data, tsize num, int root) const
      { bcastRaw (&data[0], data.size(), root); }
    template<typename T> void bcast (T &data, int root) const
      { bcastRaw (&data, 1, root); }
  };

extern MPI_Manager mpiMgr;

#endif
