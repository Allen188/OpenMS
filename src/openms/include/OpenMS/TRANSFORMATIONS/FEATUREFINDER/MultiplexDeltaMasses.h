// --------------------------------------------------------------------------
//                   OpenMS -- Open-Source Mass Spectrometry
// --------------------------------------------------------------------------
// Copyright The OpenMS Team -- Eberhard Karls University Tuebingen,
// ETH Zurich, and Freie Universitaet Berlin 2002-2015.
//
// This software is released under a three-clause BSD license:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of any author or any participating institution
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
// For a full list of authors, refer to the file AUTHORS.
// --------------------------------------------------------------------------
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL ANY OF THE AUTHORS OR THE CONTRIBUTING
// INSTITUTIONS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// --------------------------------------------------------------------------
// $Maintainer: Lars Nilse $
// $Authors: Lars Nilse $
// --------------------------------------------------------------------------

#ifndef OPENMS_TRANSFORMATIONS_FEATUREFINDER_MULTIPLEXDELTAMASSES_H
#define OPENMS_TRANSFORMATIONS_FEATUREFINDER_MULTIPLEXDELTAMASSES_H

#include <OpenMS/KERNEL/StandardTypes.h>

#include <vector>
#include <algorithm>
#include <iostream>

namespace OpenMS
{
  /**
   * @brief data structure for mass shift pattern
   * 
   * Groups of labelled peptides appear with characteristic mass shifts.
   * For example, for an Arg6 labeled SILAC peptide pair we expect to see
   * mass shifts of 0 and 6 Da. Or as second example, for a 
   * peptide pair of a dimethyl labelled sample with a single lysine
   * we will see mass shifts of 56 Da and 64 Da.
   * 28 Da (N-term) + 28 Da (K) and 34 Da (N-term) + 34 Da (K)
   * for light and heavy partners respectively.
   */
  class OPENMS_DLLAPI MultiplexDeltaMasses
  {
    public:
    
    /**
     * @brief set of labels associated with a mass shift
     * 
     * For example, a set of SILAC labels [Lys8, Lys8, Arg10] would
     * result in a +26 Da mass shift.
     */
    typedef std::multiset<String> LabelSet;

    /**
     * @brief mass shift with corresponding label set
     */
    typedef std::pair<double,LabelSet> DeltaMass;

    /**
     * @brief constructor
     */
    MultiplexDeltaMasses(std::vector<double> ms);
    
    /**
     * @brief constructor
     */
    MultiplexDeltaMasses(std::vector<DeltaMass> dm);
    
    /**
     * @brief add a mass shift
     */
    void addMassShift(double ms);
    
    /**
     * @brief add a delta mass
     */
    void addDeltaMass(DeltaMass dm);
    
    /**
     * @brief add a delta mass
     */
    void addDeltaMass(double m, std::multiset<String> ls);
    
    /**
     * @brief add a delta mass (with a label set consisting of a
     * single label)
     */
    void addDeltaMass(double m, String l);
    
    /**
     * @brief returns mass shifts
     */
    std::vector<double> getMassShifts() const;
    
    /**
     * @brief returns delta masses
     */
    std::vector<DeltaMass> getDeltaMasses() const;
    
    /**
     * @brief returns number of delta masses
     */
    unsigned getDeltaMassesCount() const;
   
    /**
     * @brief returns number of mass shifts
     */
    unsigned getMassShiftCount() const;
   
    /**
     * @brief returns delta mass at position i
     */
    DeltaMass getDeltaMassAt(int i) const;
    
    /**
     * @brief returns mass shift at position i
     */
    double getMassShiftAt(int i) const;
    
    private:
   
    /**
     * @brief mass shifts between peptides
     * (including zero mass shift for first peptide)
     */
    std::vector<double> mass_shifts_;
      
    /**
     * @brief mass shifts between peptides
     * (including zero mass shift for first peptide)
     */
    std::vector<DeltaMass> delta_masses_;
      
 };
  
}

#endif /* MULTIPLEXDELTAMASSES_H */
