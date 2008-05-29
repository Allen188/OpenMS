// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// --------------------------------------------------------------------------
//                   OpenMS Mass Spectrometry Framework
// --------------------------------------------------------------------------
//  Copyright (C) 2003-2008 -- Oliver Kohlbacher, Knut Reinert
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// --------------------------------------------------------------------------
// $Maintainer: Eva Lange $
// --------------------------------------------------------------------------


#ifndef OPENMS_ANALYSIS_MAPMATCHING_DELAUNAYPAIRFINDER_H
#define OPENMS_ANALYSIS_MAPMATCHING_DELAUNAYPAIRFINDER_H

#include <OpenMS/ANALYSIS/MAPMATCHING/BasePairFinder.h>
#include <OpenMS/SYSTEM/StopWatch.h>
#include <OpenMS/KERNEL/FeatureHandle.h>
#include <OpenMS/KERNEL/ConsensusFeature.h>

#include <CGAL/Cartesian.h>
#include <CGAL/Point_set_2.h>

#ifdef Debug_DelaunayPairFinder
#define V_(bla) std::cout << __FILE__ ":" << __LINE__ << ": " << bla << std::endl;
#else
#define V_(bla)
#endif
#define VV_(bla) V_(""#bla": " << bla)

namespace OpenMS
{
  
  /**
  @brief This class implements an element pair finding algorithm.
			
  This class implements a point pair finding algorithm.
  It offers a method to determine element pairs in two element maps,
  given two point maps and a transformation defined for the second element map (if no
  transformation is given, the pairs are found in the two original maps). 
  The pair finder also offers a method to compute consensus elements given 
  two element maps. This algorithm is similar to the pair finding method as mentioned above,
  but it implies that the scene map is already dewarped.
			     
  To speed up the search for element pairs an consensus elements, the DelaunayPairFinder
  uses the CGAL delaunay triangulation for the nearest neighbour search.
			
  The template parameter is the type of the consensus map.
			
  @note The RT and the MZ dimension are not equivalent, because two elements that differ in RT by 1s (or minute) are 
  more similar than two points that differ in MZ by 1Th. To be able to use the euclidean distance in the nearest neighbour search, 
  we have to transform the elements MZ position m into a new MZ position m'= m * internal_mz_scaling;
  E.g. given internal_mz_scaling=10 results in 1s difference in RT being similar to 0.1Th difference in MZ.
			 
  @ref DelaunayPairFinder_Parameters are explained on a separate page.  

  @todo work out all TODOs in the code

  @ingroup FeatureGrouping
  */
  class DelaunayPairFinder : public BasePairFinder
  {
   public:

    typedef BasePairFinder Base;

    /// Constructor
    DelaunayPairFinder() : Base()
    {
      //set the name for DefaultParamHandler error messages
      Base::setName(getProductName());

      defaults_.setValue("similarity:max_pair_distance:RT", 20.0, "Maximal allowed distance in retention time for a pair to be matched");
      defaults_.setValue("similarity:max_pair_distance:MZ",  1.0, "Maximal allowed distance in mass-to-charge for a pair to be matched");
      defaults_.setValue("similarity:second_nearest_gap",    0.0, "The distance of the second nearest neighbors must be this factor larger than to the nearest");

      Base::defaultsToParam_();
    }

    /// Destructor
    virtual ~DelaunayPairFinder()
    {}

    /// Returns an instance of this class
    static BasePairFinder* create()
    {
      return new DelaunayPairFinder();
    }

    /// Returns the name of this module
    static const String getProductName()
    {
      return "delaunay";
    }

    /**@brief Nested class, which inherits from the CGAL Point_2 class and
    additionally contains a reference to the corresponding element and a
    unique key

    @todo check which ctors are really needed and make sense
    */
    struct Point : public CGAL::Point_2< CGAL::Cartesian<double> >
    {
      typedef CGAL::Point_2< CGAL::Cartesian<double> > Base;

      const ConsensusFeature* element;
      Int key;
			
      /// Default ctor
      inline Point() : Base(), element(0), key(0) {}
			
      /// Ctor from Base class, aka CGAL:Point_2<...>
      inline Point(const Base& cgal_point) : Base(cgal_point), element(0), key(0) {}

      /// Ctor from coordinates, element, and key
      inline Point(double hx, double hy, const ConsensusFeature& element, Int key) : Base(hx,hy), element(&element), key(key) {}

      /// Ctor from coordinates
      inline Point(double hx, double hy) : Base(hx,hy), element(0), key(0) {}

      /// Dtor
      ~Point() {}

      /// Copy ctor
      Point(const Point& rhs) : Base(rhs), element(rhs.element) , key(rhs.key){}

      ///  Assignment operator
      Point& operator = (const Point& source)
      {
        if (this==&source) return *this;
        Base::operator=(source);
        element = source.element;
        key = source.key;
        return *this;
      }
    };

    /// To construct a Delaunay triangulation with our Point class we have to
    /// write an own geometric traits class and the operator() (that generates
    /// a Point given a CGAL circle)
    class  GeometricTraits : public CGAL::Cartesian<double>
    {
     public:
      typedef Point Point_2;
      ///
      class Construct_center_2
      {
        typedef Point   Point_2;
        typedef CGAL::Cartesian<double>::Circle_2  Circle_2;
       public:
        typedef Point_2          result_type;
        typedef CGAL::Arity_tag< 1 >   Arity;

        Point_2
        operator()(const Circle_2& c) const
        {
          return c.center();
        }
      };
    };

    typedef CGAL::Point_set_2< GeometricTraits, CGAL::Triangulation_data_structure_2< CGAL::Triangulation_vertex_base_2< GeometricTraits > > > Point_set_2;
    typedef Point_set_2::Vertex_handle Vertex_handle;

		/**@brief Just a silly array of Point of size two which has constructors
		missing in Point[2].  (std::vector<Point[2]> doesn't work.)
		*/
		struct PointArray2
		{
			/// default ctor
			PointArray2() {array_[0] = Point(); array_[1] = Point();}
			/// ctor from two Ints
			PointArray2(Point const i0, Point const i1 ) {array_[0] = i0; array_[1] = i1;}
			/// copy ctor
			PointArray2(PointArray2 const& rhs) {array_[0] = rhs.array_[0]; array_[1] = rhs.array_[1];}
			/// assignment op
			PointArray2 & operator=(PointArray2 const& rhs) {array_[0] = rhs.array_[0]; array_[1] = rhs.array_[1]; return *this;}
			/// indexing op
			Point & operator[](UInt const index) {return array_[index];}
			/// indexing op
			Point const& operator[](UInt const index) const {return array_[index];}

		 protected:
			/// the underlying array
			Point array_[2];
		};

    /// documented in base class
    void run(ConsensusMap &result_map)
    {
      V_("@@@ DelaunayPairFinder::run()");
			VV_(max_pair_distance_[RT]);
			VV_(max_pair_distance_[MZ]);
			VV_(internal_mz_scaling_);
			VV_(max_squared_distance_);
			VV_(second_nearest_gap_);


      // Every derived class should set maps_.result_ at the beginning of run()
      maps_.result_ = &result_map;

			// The delaunay triangulation data structures for model and scene.
			Point_set_2 p_set[2];

			// We will add two outlier points to each p_set so that we will always
			// have at least three points in the Delaunay triangulation and the
			// nearest neighbor search will succeed.  [NOTE about
			// really_big_doublereal: Something like
			// std::numeric_limits<DoubleReal>::max() / 2.  does not work here,
			// because CGAL fails on a precondition.  But 1E10 is a really big
			// number, doesn't it?  Found by trial and error.  Clemens, 2008-05-19]
			DoubleReal const really_big_doublereal = 1E10;
			Feature outlier_feature_1;
			outlier_feature_1.setRT(-really_big_doublereal); outlier_feature_1.setMZ(-really_big_doublereal);
			ConsensusFeature const outlier_consensusfeature_1( std::numeric_limits<UInt>::max(), std::numeric_limits<UInt>::max(), outlier_feature_1 );
			Point const outlier_point_1( -really_big_doublereal, -really_big_doublereal, outlier_consensusfeature_1, -1 );
			Feature outlier_feature_2;
			outlier_feature_2.setRT(really_big_doublereal); outlier_feature_2.setMZ(really_big_doublereal);
			ConsensusFeature const outlier_consensusfeature_2( std::numeric_limits<UInt>::max(), std::numeric_limits<UInt>::max(), outlier_feature_2 );
			Point const outlier_point_2( really_big_doublereal, really_big_doublereal, outlier_consensusfeature_2, -1 );
			PointArray2 const outlier_points( outlier_point_1, outlier_point_2 );
						
			// do the preprocessing for both input maps
      for ( UInt input = MODEL_; input <= SCENE_; ++ input )
      {
				// Check whether map index is meaningful
				if ( map_index_array_[input] < -1 )
				{
					throw Exception::OutOfRange(__FILE__,__LINE__,__PRETTY_FUNCTION__);
				}
				// result must not overwrite input
				if (  maps_.result_ == maps_array_[input] )
				{
					throw Exception::IllegalSelfOperation(__FILE__,__LINE__,__PRETTY_FUNCTION__);
				}

				// TODO Find out whether it is (1) correct and (2) fast if we
				// push_back() the Points into the Delaunay triangulation. Otherwise,
				// use an iterator adapter and construct Point_set_2 p_set from an
				// iterator range.
				if ( input == MODEL_ )
				{
					for (UInt i = 0; i < getModelMap().size(); ++i)
					{
						DoubleReal trans_rt = getModelMap()[i].getRT();
						DoubleReal trans_mz = getModelMap()[i].getMZ() * internal_mz_scaling_;
						p_set[MODEL_].push_back( Point( trans_rt, trans_mz, getModelMap()[i], i) );
						V_("MODEL_: trans_rt:"<<trans_rt<<" trans_mz:"<<trans_mz);
					}
				}
				else // input == SCENE_
				{
					V_("Transformation rt " << transformation_[RawDataPoint2D::RT]);
					for (UInt i = 0; i < getSceneMap().size(); ++i)
					{
						DoubleReal trans_rt = getSceneMap()[i].getRT();
						transformation_[RawDataPoint2D::RT].apply(trans_rt);
						DoubleReal trans_mz = getSceneMap()[i].getMZ() * internal_mz_scaling_;
						p_set[SCENE_].push_back( Point( trans_rt, trans_mz, getSceneMap()[i], i) );
						V_("SCENE_: trans_rt:"<<trans_rt<<" trans_mz:"<<trans_mz);
					}
				}
				
				p_set[input].push_back(outlier_point_1);
				p_set[input].push_back(outlier_point_2);
				V_("p_set[" << input << "].number_of_vertices(): " << p_set[input].number_of_vertices() << "  [includes two dummy outliers]");
      }

      // Empty output destination
      result_map.clear();

			// In this we store the best ([0]) and second best ([1]) neighbours;
			// essentially this is a regular bipartite graph of degree two.
			// E.g. neighbours[MODEL_][i][j] is the j-th best match (index of an
			// element of scene) for the i-th consensus feature in the model.
			std::vector<PointArray2> neighbours[2];
			
			// For each input, we find nearest and second nearest neighbors in the
			// other map.
			std::vector<Vertex_handle> neighbors_buffer;
      for ( UInt input = MODEL_; input <= SCENE_; ++ input )
      {
				UInt const other_input = 1 - input;
				
				neighbours[input].resize( maps_array_[input]->size(), outlier_points );

				for ( Point_set_2::Point_iterator iter = p_set[input].points_begin(); iter != p_set[input].points_end(); ++iter )
				{
					if ( iter-> key == -1 ) continue;
					neighbors_buffer.clear();
					VV_(iter->key);
					p_set[other_input].nearest_neighbors( *iter, 2, std::back_inserter(neighbors_buffer) );
					neighbours[input][iter->key][0] = neighbors_buffer[0]->point();
					neighbours[input][iter->key][1] = neighbors_buffer[1]->point();
					VV_(neighbours[input][iter->key][0].key);
					VV_(neighbours[input][iter->key][1].key);
					V_( iter->x() << " " << iter->y() << " " << 
							neighbours[input][iter->key][0].x() << " " << neighbours[input][iter->key][0].y() << " " <<
							neighbours[input][iter->key][1].x() << " " << neighbours[input][iter->key][1].y() );
				}
			}
			
      // Initialize a hash map for the elements of model map to avoid that
      // elements of the reference map occur in several element pairs
			//
      // The semantics for the hashed values is:
      // -1: not touched,
      // -2: cannot assign unambiguously, (currently not being used!)
      // >=0: index of the matching scene ConsensusFeature
      std::vector<Int> matches[2];
			matches[MODEL_].resize(getModelMap().size(),-1);
			matches[SCENE_].resize(getSceneMap().size(),-1);
			
			V_("max_pair_distance_[RT]: " << max_pair_distance_[RT]);
			V_("max_pair_distance_[MZ]: " << max_pair_distance_[MZ]);
			V_("max_squared_distance_:  " << max_squared_distance_ );
			V_("second_nearest_gap:     " << second_nearest_gap_   );

      UInt current_result_cf_index = 0;

      // take each point in the model map and search for its neighbours in the scene map
			
			// for ( Int model_cf_index = 0; model_cf_index < (Int)getModelMap().size(); ++model_cf_index )
			for ( Point_set_2::Point_iterator model_iter = p_set[MODEL_].points_begin(); model_iter != p_set[MODEL_].points_end(); ++model_iter )
      {
				Int const model_cf_index = model_iter->key;
				if ( model_cf_index == -1 ) continue;
				
				Point const & scene_point = neighbours[MODEL_][model_cf_index][0];
				Int const scene_cf_index = scene_point.key;
				if ( scene_cf_index == -1 ) continue;

				VV_(model_cf_index);
				VV_(scene_cf_index);

				bool const is_bidirectionally_nearest = neighbours[SCENE_][ scene_cf_index ][0].key == model_cf_index;
				VV_(is_bidirectionally_nearest);

				if ( is_bidirectionally_nearest )
				{
					DoubleReal pair_distance_m_s =
						squared_distance_( model_iter->x(), model_iter->y(),
															 scene_point.x(), scene_point.y()  );
					
					VV_(pair_distance_m_s);
					bool const is_close_enough = ( pair_distance_m_s <= max_squared_distance_ );
					VV_(is_close_enough);

					if ( is_close_enough ) /* && is_bidirectionally_nearest */
					{
						Point const & scene_point_second_nearest = neighbours[MODEL_][model_cf_index][1];
						DoubleReal pair_distance_m_s2nd =
							squared_distance_( model_iter->x(),                model_iter->y(),
																 scene_point_second_nearest.x(), scene_point_second_nearest.y() );
						VV_(pair_distance_m_s2nd);
						Point const & model_point_second_nearest = neighbours[SCENE_][scene_cf_index][1];
						VV_(model_point_second_nearest);
						DoubleReal pair_distance_m2nd_s =
							squared_distance_( model_point_second_nearest.x(), model_point_second_nearest.y(),
																 scene_point.x(),                scene_point.y()                 );
						VV_(pair_distance_m2nd_s);
						DoubleReal min_second_pair_distance = pair_distance_m_s * second_nearest_gap_;
						VV_(min_second_pair_distance);
						bool const is_unambiguous = 
							( pair_distance_m_s2nd >= min_second_pair_distance &&
								pair_distance_m2nd_s >= min_second_pair_distance    );
						VV_(is_unambiguous);
						if ( is_unambiguous ) /* && is_close_enough && is_bidirectionally_nearest */
						{
							// assign matching pair
							matches[MODEL_][model_cf_index] = scene_cf_index;
							matches[SCENE_][scene_cf_index] = model_cf_index;

							VV_(matches[MODEL_][model_cf_index]);
							VV_(matches[SCENE_][scene_cf_index]);
							VV_(current_result_cf_index);

							// create a consensus feature
							/* TODO: optionally apply the transformation to scene (DISCUSS:
							deep or shallow?) -- see also next comment below */
							maps_.result_->push_back(ConsensusFeature());
							if ( map_index_.scene_ == -1 )
							{
								maps_.result_->back().insert( getSceneMap()[scene_cf_index] );
							}
							else
							{
								maps_.result_->back().insert( map_index_.scene_, scene_cf_index, getSceneMap()[scene_cf_index] );
							}
							/* 
							if ( warp_scene_in_result_ ) { ... transform what is already in the consensus feature ... }
							*/
							if ( map_index_.model_ == -1 )
							{
								maps_.result_->back().insert( getModelMap()[model_cf_index] );
							}
							else
							{
								maps_.result_->back().insert( map_index_.model_, model_cf_index, getModelMap()[model_cf_index] );
							}
							maps_.result_->back().computeConsensus();
							V_("Result " << current_result_cf_index << " : " << maps_.result_->back());
							++current_result_cf_index;
						}
					}
				}
      }

      // write out singleton consensus features for unmatched consensus
      // features in model and scene // TODO optionally transform scene
      for ( UInt input = MODEL_; input <= SCENE_; ++ input )
      {
				for ( UInt index = 0; index < maps_array_[input]->size(); ++ index )
				{
					if ( matches[input][index] < 0 )
					{
						maps_.result_->push_back(ConsensusFeature());
						if ( map_index_array_[input] == -1)
						{
							maps_.result_->back().insert( (*maps_array_[input])[index] );
						}
						else
						{
							maps_.result_->back().insert( map_index_array_[input], index, (*maps_array_[input])[index] );
						}
						maps_.result_->back().computeConsensus();
						V_("Result " << current_result_cf_index << " : " << maps_.result_->back());
						V_("matches["<<input<<"]["<<index<< "]: " << matches[input][index] );
						++current_result_cf_index;
					}
				}
			}
			
			// Very useful for checking the results :-)
			maps_.result_->sortByNthPosition(RawDataPoint2D::MZ);
			
      return;
    }

   protected:

		DoubleReal squared_distance_( DoubleReal x1, DoubleReal y1, DoubleReal x2, DoubleReal y2 ) const
		{
			return pow(x1-x2,2) + pow(y1-y2,2); // TODO: check if pow(x,2) is really faster than x*x  (as claimed by AnHi)
		}
		
		virtual void updateMembers_()
    {
			max_pair_distance_[RT] = (DoubleReal) param_.getValue("similarity:max_pair_distance:RT");
			max_pair_distance_[MZ] = (DoubleReal) param_.getValue("similarity:max_pair_distance:MZ");
			internal_mz_scaling_   = max_pair_distance_[RT] / max_pair_distance_[MZ];
			max_squared_distance_  = pow(max_pair_distance_[RT],2);
      second_nearest_gap_    = (DoubleReal) param_.getValue("similarity:second_nearest_gap");

			V_("@@@ DelaunayPairFinder::updateMembers_()");
			VV_(max_pair_distance_[MZ]);
			VV_(internal_mz_scaling_);
			VV_(max_squared_distance_);
			VV_(second_nearest_gap_);

      return;
    }
		
		enum  { RT = RawDataPoint2D::RT, MZ = RawDataPoint2D::MZ };

		/// Maximal distance of a matched pair, in both dimension RT and MZ
    DoubleReal max_pair_distance_[2];

    /// Factor by which MZ has to be rescaled so that differences in MZ and RT are equally significant.
    DoubleReal internal_mz_scaling_;

		/// Upper bound for squared_distance_()
		DoubleReal max_squared_distance_;
		
		/// The distance of the second nearest neighbor must be this factor larger
    DoubleReal second_nearest_gap_;

  }; // class

} // namespace OpenMS

#undef V_

#endif  // OPENMS_ANALYSIS_MAPMATCHING_DELAUNAYPAIRFINDER_H
