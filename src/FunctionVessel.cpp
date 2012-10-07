/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   Copyright (c) 2012 The plumed team
   (see the PEOPLE file at the root of the distribution for a list of names)

   See http://www.plumed-code.org for more information.

   This file is part of plumed, version 2.0.

   plumed is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   plumed is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with plumed.  If not, see <http://www.gnu.org/licenses/>.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
#include "FunctionVessel.h"
#include "ActionWithDistribution.h"

namespace PLMD {

SumVessel::SumVessel( const VesselOptions& da ):
VesselAccumulator(da)
{
}

bool SumVessel::calculate( const unsigned& icv, const double& tolerance ){
  bool keep=false; double f, df; unsigned jout;
  Value myval=getAction()->retreiveLastCalculatedValue();
  for(unsigned j=0;j<getNumberOfValues();++j){
      f=compute( j, myval.get(), df );
      if( fabs(f)>tolerance ){
          keep=true; 
          jout=value_starts[j]; 
          addToBufferElement( jout, f ); jout++;
          getAction()->mergeDerivatives( icv, myval, df, jout, this );        
      }  
  }
  return keep;
}

double SumVessel::final_computations( const unsigned& ival, const double& valin, double& df ){
  df=1; return valin; 
}

void SumVessel::finish( const double& tolerance ){
  double f, df;
  for(unsigned i=0;i<getNumberOfValues();++i){
      getValue( i, myvalue2 ); 
      f=final_computations( i, myvalue2.get(), df );
      myvalue2.chainRule(df); myvalue2.set(f);
      copy( myvalue2, getPntrToOutput(i) );
  }
}

NormedSumVessel::NormedSumVessel( const VesselOptions& da ):
VesselAccumulator(da),
donorm(false)
{
  if( getAction()->isPeriodic() ){
      double min, max;
      getAction()->retrieveDomain( min, max );
      myvalue2.setDomain( min, max );
  } else {
      myvalue2.setNotPeriodic();
  }
}

void NormedSumVessel::useNorm(){
  donorm=true; addBufferedValue();
}

bool NormedSumVessel::calculate( const unsigned& icv, const double& tolerance ){
  bool keep=false;
  if(donorm){
     getWeight( icv, myweight );
     if( myweight.get()>tolerance ){
         keep=true; 
         addToBufferElement( 0, myweight.get() ); 
         getAction()->mergeDerivatives( icv, myweight, 1.0, 1, this );
     }
     if(!keep) return false;

     unsigned jout;
     for(unsigned j=1;j<getNumberOfValues()+1;++j){
        compute( icv, j-1, myvalue );
        if( fabs( myvalue.get() )>tolerance ){
            keep=true; 
            jout=value_starts[j]; 
            addToBufferElement( jout, myvalue.get() ); jout++;
            getAction()->mergeDerivatives( icv, myvalue, 1.0, jout, this );
        }  
     }
  } else {
     unsigned jout;
     for(unsigned j=0;j<getNumberOfValues();++j){
        compute( icv, j, myvalue );
        if( myvalue.get()>tolerance ){
            keep=true; 
            jout=value_starts[j]; 
            addToBufferElement( jout, myvalue.get() ); jout++;
            getAction()->mergeDerivatives( icv, myvalue, 1.0, jout, this );
        }
     }   
  }
  return keep;
}

void NormedSumVessel::finish( const double& tolerance ){
  if( donorm ){
     getValue(0, myweight2 ); 
     for(unsigned i=0;i<getNumberOfValues();++i){
         getValue( i+1, myvalue2 );       /// ARSE periodicity
         quotient( myvalue2, myweight2, getPntrToOutput(i) );
     }
  } else {
     for(unsigned i=0;i<getNumberOfValues();++i){
         getValue( i, myvalue2 ); copy( myvalue2, getPntrToOutput(i) );
     }
  }
}

}
