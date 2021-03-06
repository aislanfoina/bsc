/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                             ClusteringSuite                               *
 *   Infrastructure and tools to apply clustering analysis to Paraver and    *
 *                              Dimemas traces                               *
 *                                                                           * 
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\

  $URL:: https://svn.bsc.#$:  File
  $Rev:: 20               $:  Revision of last commit
  $Author$:  Author of last commit
  $Date$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _POINT_HPP_
#define _POINT_HPP_

#include <vector>
using std::vector;

class Point
{
  protected:
    vector<double> Dimensions;

    bool Normalized;
    
  public:

    Point(size_t Dimensions);

    Point(vector<double>& _Dimensions);

    void   RangeNormalization(const vector<double>& MaxValues,
                              const vector<double>& MinValues);

    void   ScaleDimensions(const vector<double>& Factors);
    
    double EuclideanDistance(Point& OtherPoint) const;
    
    double NormalizedEuclideanDistance(Point& OtherPoint) const;

    bool   IsNormalized(void) const { return Normalized; };

    size_t size(void) const;

    double& operator[](int i);
    
    const double operator[](int i) const;

    void check_const(void) { Normalized = true;};

    void PrintPoint(void);
    
  private:

};

#endif // _POINT_HPP_
