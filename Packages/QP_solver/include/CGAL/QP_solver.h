// ============================================================================
//
// Copyright (c) 1997-2004 The CGAL Consortium
//
// This software and related documentation is part of an INTERNAL release
// of the Computational Geometry Algorithms Library (CGAL). It is not
// intended for general use.
//
// ----------------------------------------------------------------------------
//
// release       : $CGAL_Revision: CGAL-I $
// release_date  : $CGAL_Date$
//
// file          : include/CGAL/QPE_solver.h
// package       : $CGAL_Package: QP_engine $
// chapter       : Quadratic Programming Engine
//
// revision      : 3.0alpha
// revision_date : 2004/06
//
// author(s)     : Sven Sch�nherr <sven@inf.ethz.ch>
// coordinator   : ETH Z�rich (Bernd G�rtner <gaertner@inf.ethz.ch>)
//
// implementation: Quadratic Programming Engine - Solver
// ============================================================================

#ifndef CGAL_QPE_SOLVER_H
#define CGAL_QPE_SOLVER_H

// includes
#ifndef CGAL_QP_ENGINE_BASIC_H
#include <CGAL/QP_engine/basic.h>
#endif
#ifndef CGAL_QP_ENGINE_ITERATOR_H
#include <CGAL/QP_engine/iterator.h>
#endif
#ifndef CGAL_QP_ENGINE_FUNCTORS_H
#include <CGAL/QP_engine/functors.h>
#endif
#ifndef CGAL_QP_ENGINE_QPE_BASIS_INVERSE_H
#include <CGAL/QP_engine/QPE_basis_inverse.h>
#endif
#ifndef CGAL_QPE_PRICING_STRATEGY_H
#include <CGAL/QPE_pricing_strategy.h>
#endif


#include <CGAL/functional.h>
#include <CGAL/algorithm.h>

#ifndef CGAL_IO_VERBOSE_OSTREAM_H
#include <CGAL/IO/Verbose_ostream.h>
#endif

#ifndef CGAL_PROTECT_VECTOR
#  define CGAL_PROTECT_VECTOR
#  include <vector>
#endif
#ifndef CGAL_PROTECT_NUMERIC
#  define CGAL_PROTECT_NUMERIC
#  include <numeric>
#endif
#ifndef CGAL_PROTECT_ALGORITHM
#  define CGAL_PROTECT_ALGORITHM
#  include <algorithm>
#endif

CGAL_BEGIN_NAMESPACE

// =================
// class declaration
// =================
template < class Rep_ >
class QPE_solver;

// ===============
// class interface
// ===============
template < class Rep_ >
class QPE_solver {

  public:

    // self
    typedef  Rep_                       Rep;
    typedef  QPE_solver<Rep>            Self;

    // types from the representation class
    typedef  typename Rep::ET           ET;

    typedef  typename Rep::A_iterator   A_iterator;
    typedef  typename Rep::B_iterator   B_iterator;
    typedef  typename Rep::C_iterator   C_iterator;
    typedef  typename Rep::D_iterator   D_iterator;

    typedef  typename Rep::Row_type     Row_type;
    typedef  typename Rep::Row_type_iterator
                                        Row_type_iterator;

    typedef  typename Rep::Is_linear    Is_linear;
    typedef  typename Rep::Is_symmetric Is_symmetric;
    typedef  typename Rep::Has_no_inequalities
                                        Has_no_inequalities;

  private:

    // private types
    // -------------
    // types of original problem
    typedef  typename std::iterator_traits<A_iterator>::value_type  A_column;
    typedef  typename std::iterator_traits<D_iterator>::value_type  D_row;

    typedef  typename std::iterator_traits<A_column  >::value_type  A_entry;
    typedef  typename std::iterator_traits<B_iterator>::value_type  B_entry;
    typedef  typename std::iterator_traits<C_iterator>::value_type  C_entry;
    typedef  typename std::iterator_traits<D_row     >::value_type  D_entry;

    // slack columns
    typedef  std::pair<int,bool>        Slack_column;
    typedef  std::vector<Slack_column>  A_slack;

    // artificial columns
    typedef  std::pair<int,bool>        Art_column;
    typedef  std::vector<Art_column>    A_art;
    typedef  std::vector<A_entry>       S_art;

    // auxiliary objective vector
    typedef  std::vector<C_entry>       C_aux;
    

    // indices (variables and constraints)
public:
    // QPE__partial_base.h needs them
    typedef  std::vector<int>           Indices;

    // it seems we need the non-const version here as well
    typedef  Indices::iterator            Index_iterator;
    typedef  Indices::const_iterator      Index_const_iterator;
    //typedef  Indices::const_iterator    Index_iterator;

private:
    // values (variables' numerators)
    typedef  std::vector<ET>            Values;
    typedef  typename Values::iterator  Value_iterator;
    typedef  typename Values::const_iterator
                                        Value_const_iterator;

    // quotient functor
    
    typedef  Creator_2< ET, ET, Quotient<ET> >
                                        Quotient_creator;
    
    
 //   typedef  std::binder2nd< Quotient_creator >
 //                                       Quotient_maker;
    typedef  typename CGAL::Bind<Quotient_creator,
                 typename Quotient_creator::argument2_type,2>::Type
                                        Quotient_maker;
    
    
    // access values by basic index functor
    typedef  Value_by_basic_index<Value_const_iterator>
                                        Value_by_basic_index;

    // access to original problem by basic variable/constraint index
    typedef  QPE_vector_accessor<
		 typename std::iterator_traits<A_iterator>::value_type,
		 false, false >         A_by_index_accessor;
    typedef  QPE_transform_iterator_1< Index_const_iterator, A_by_index_accessor >
                                        A_by_index_iterator;

    typedef  QPE_vector_accessor< B_iterator, false, false >
                                        B_by_index_accessor;
    typedef  QPE_transform_iterator_1< Index_const_iterator, B_by_index_accessor >
                                        B_by_index_iterator;

    typedef  QPE_vector_accessor< C_iterator, false, false >
                                        C_by_index_accessor;
    typedef  QPE_transform_iterator_1< Index_const_iterator, C_by_index_accessor >
                                        C_by_index_iterator;

    typedef  QPE_vector_accessor<
		 typename std::iterator_traits<D_iterator>::value_type,
		 false, false >         D_by_index_accessor;
    typedef  QPE_transform_iterator_1< Index_const_iterator, D_by_index_accessor >
                                        D_by_index_iterator;

    typedef  QPE_matrix_accessor< A_iterator, false, true, false, false>
                                        A_accessor;
    typedef  typename CGAL::Bind< A_accessor,
    	typename A_accessor::argument2_type,2>::Type
                                        A_row_by_index_accessor;
 //   typedef  std::binder2nd< A_accessor >
 //                                       A_row_by_index_accessor;
    typedef  QPE_transform_iterator_1<
		 Index_iterator, A_row_by_index_accessor >
                                        A_row_by_index_iterator;

    typedef  QPE_matrix_pairwise_accessor< D_iterator, Is_symmetric >
                                        D_pairwise_accessor;
    typedef  QPE_transform_iterator_1< Index_const_iterator, D_pairwise_accessor >
                                        D_pairwise_iterator;

    // access to special artificial column by basic constraint index
    typedef  QPE_vector_accessor< typename S_art::const_iterator, false, false>
                                        S_by_index_accessor;
    typedef  QPE_transform_iterator_1< Index_iterator, S_by_index_accessor >
                                        S_by_index_iterator;

  public:

    // public types
    enum Status { UPDATE, INFEASIBLE, UNBOUNDED, OPTIMAL };
    
    typedef  typename A_slack::const_iterator
                                        A_slack_iterator;

    typedef  typename A_art::const_iterator
                                        A_artificial_iterator;
    
    typedef  typename C_aux::const_iterator
                                        C_auxiliary_iterator;

    typedef  QPE_transform_iterator_1<
                 Index_const_iterator, Value_by_basic_index >
                                        Variable_numerator_iterator;
    typedef  QPE_transform_iterator_1<
                 Variable_numerator_iterator, Quotient_maker >
                                        Variable_value_iterator;
    /*
    typedef  Variable_numerator_iterator
                                        Working_variable_numerator_iterator;
    typedef  Variable_value_iterator    Working_variable_value_iterator;
    */

    typedef  Index_const_iterator       Basic_variable_index_iterator;
    typedef  Value_const_iterator       Basic_variable_numerator_iterator;
    typedef  QPE_transform_iterator_1<
                 Basic_variable_numerator_iterator, Quotient_maker >
                                        Basic_variable_value_iterator;
    
    typedef  Index_const_iterator       Basic_constraint_index_iterator;
    
    typedef  Value_const_iterator       Lambda_numerator_iterator;
    typedef  QPE_transform_iterator_1<
                 Lambda_numerator_iterator, Quotient_maker >
                                        Lambda_value_iterator;
    
    typedef  QPE_pricing_strategy<Rep>  Pricing_strategy;

  private:
    Tag_false                is_perturbed;
    // some constants
    const ET                 et0, et1, et2;

    // verbose output streams
    Verbose_ostream          vout;      // used for any  diagnostic output
    Verbose_ostream          vout1;     // used for some diagnostic output
    Verbose_ostream          vout2;     // used for more diagnostic output
    Verbose_ostream          vout3;     // used for full diagnostic output
    Verbose_ostream          vout4;     // used for output of basis inverse
    Verbose_ostream	     vout5; 	// used for output of validity tests
    
    // pricing strategy
    Pricing_strategy*        strategyP;
    
    // given QP
    int                      qp_n;      // number of variables
    int                      qp_m;      // number of constraints
    
    A_iterator               qp_A;      // constraint matrix
    B_iterator               qp_b;      // right-hand-side vector
    C_iterator               qp_c;      // objective vector
    D_iterator               qp_D;      // objective matrix
    Row_type_iterator        qp_r;      // row-types of constraints

    A_slack                  slack_A;   // slack part of constraint matrix

    // auxiliary problem    
    A_art                    art_A;     // artificial part of constraint matrix
    S_art                    art_s;     // special artificial column for slacks
    int                      art_s_i;   // art_s_i>=0 ->index of special artificial
    					// column
					// art_s_i == -1 -> no sp. art. col
					// art_s_i == -2 -> sp. art. col removed
					// after it left basis 
    int                      art_basic; // number of basic artificial variables
    C_aux                    aux_c;     // objective function for phase I
    					// initially has the same size as A_art
    
    // current status
    Indices                  B_O;       // basis (original variables)
    Indices                  B_S;       // basis (   slack variables)
    
    Indices                  C;         //    basic constraints ( C = E+S_N )
    Indices                  S_B;       // nonbasic constraints ( S_B '=' B_S)
    
    QPE_basis_inverse<ET,Is_linear>
                             inv_M_B;   // inverse of basis matrix

    const ET&                d;         // reference to `inv_M_B.denominator()'
    
    Values                   x_B_O;     // basic variables (original)
    Values                   x_B_S;     // basic variables (slack)
    Values                   lambda;    // lambda (from KKT conditions)
    
    int                      m_phase;   // phase of the Simplex method
    Status                   m_status;  // status of last pivot step
    int                      m_pivots;  // number of pivot steps
    
    bool                     is_phaseI; // flag indicating phase I
    bool                     is_phaseII;// flag indicating phase II
    bool                     is_RTS_transition; // flag indicating transition
                                        // from Ratio Test Step1 to Ratio
                                        // Test Step2                                           
    const bool               is_LP;     // flag indicating a linear    program
    const bool               is_QP;     // flag indicating a quadratic program
    const bool                no_ineq;  // flag indicating no ineq. constraits
    const bool               has_ineq;  // flag indicating    ineq. constraits

    // additional variables
    int                      l;         // minimum of 'qp_n+e+1' and 'qp_m'
    
    int 		     e;         // number of equality constraints
    
    Indices                  in_B;      // variable   in basis, -1 if non-basic
    Indices                  in_C;      // constraint in basis, -1 if non-basic

    Values                   b_C;       // exact version of `qp_b'
                                        // restricted to basic constraints C
    Values                   minus_c_B; // exact version of `-qp_c'
                                        // restricted to basic variables B_O

    Values                   A_Cj;      // exact version of j-th column of A
                                        // restricted to basic constraints C
    Values                   two_D_Bj;  // exact version of twice the j-th
                                        // column of D restricted to B_O
    
    int                      j;         // index of entering variable `x_j'
    
    int                      i;         // index of leaving variable `x_i'
    ET                       x_i;       // numerator of leaving variable `x_i'
    ET                       q_i;       // corresponding `q_i'

    ET                       mu;        //   numerator of `t_j'
    ET                       nu;        // denominator of `t_j'
    
    Values                   q_lambda;  // length dependent on C
    Values                   q_x_O;     // used in the ratio test & update
    					// length dependent on B_O
    Values                   q_x_S;     // 

    Values                   tmp_l;     // temporary vector of size l
    Values                   tmp_x;     // temporary vector of s. >= B_O.size()
    Values                   tmp_l_2;   // temporary vector of size l
    Values                   tmp_x_2;   // temporary vector of s. >= B_O.size()

  public:

    /*
     * Note: Some member functions below are suffixed with '_'.
     * They are member templates and their declaration is "hidden",
     * because they are also implemented in the class interface.
     * This is a workaround for M$-VC++, which otherwise fails to
     * instantiate them correctly.
     */

    // creation & initialization
    // -------------------------
    // creation
    QPE_solver( );

    // set-up of QP
    void  set( int n, int m,
	       A_iterator A, B_iterator b, C_iterator c, D_iterator D,
	       Row_type_iterator r
	           = QPE_const_value_iterator<Row_type>( Rep::EQUAL));

    // initialization (of phase I)
    void  init( );
    
    // initialization (of phase II)
    /*
    template < class InputIterator >
    void  init( InputIterator  basic_variables_first,
                InputIterator  basic_variables_beyond);
    */
    
    // operations
    // ----------
    // pivot step
    Status  pivot( )
        { CGAL_qpe_precondition( phase() > 0);
          CGAL_qpe_precondition( phase() < 3);
          pivot_step();
          return status(); }

    // solve QP
    Status  solve( )
        { CGAL_qpe_precondition( phase() > 0);
          while ( phase() < 3) { pivot_step(); }
          return status(); }

    // access
    // ------
    // access to QP
    int  number_of_variables  ( ) const { return qp_n; }
    int  number_of_constraints( ) const { return qp_m; }
    
    A_iterator  a_begin( ) const { return qp_A;      }
    A_iterator  a_end  ( ) const { return qp_A+qp_n; }
    
    B_iterator  b_begin( ) const { return qp_b;      }
    B_iterator  b_end  ( ) const { return qp_b+qp_m; }
    
    C_iterator  c_begin( ) const { return qp_c;      }
    C_iterator  c_end  ( ) const { return qp_c+qp_n; }
    
    D_iterator  d_begin( ) const { return qp_D;      }
    D_iterator  d_end  ( ) const { return qp_D+qp_n; }
    
    Row_type_iterator  row_type_begin( ) const { return qp_r;      }
    Row_type_iterator  row_type_end  ( ) const { return qp_r+qp_m; }

    // access to current status
    int     phase     ( ) const { return m_phase;  }
    Status  status    ( ) const { return m_status; }
    int     iterations( ) const { return m_pivots; }
    
    // access to common denominator
    const ET&  variables_common_denominator( ) const { return d; }

    // access to current solution
    ET  solution_numerator( ) const;
    
    Quotient<ET>  solution( ) const
        { return Quotient<ET>( solution_numerator(), d*d); }

    // access to original variables
    int  number_of_original_variables( ) const { return qp_n; }

    Variable_numerator_iterator
    variables_numerator_begin( ) const
        { return Variable_numerator_iterator( in_B.begin(),
                   Value_by_basic_index( x_B_O.begin(), qp_n, x_B_S.begin()));}
    
    Variable_numerator_iterator
    variables_numerator_end  ( ) const
        { return Variable_numerator_iterator( in_B.end(),
                   Value_by_basic_index( x_B_O.begin(), qp_n, x_B_S.begin()));}
    
    Variable_value_iterator
    variables_value_begin( ) const
        { return Variable_value_iterator(
                     variables_numerator_begin(),
                     Quotient_maker( Quotient_creator(), d)); }
    
    Variable_value_iterator
    variables_value_end  ( ) const
        { return Variable_value_iterator(
                     variables_numerator_end(),
                     Quotient_maker( Quotient_creator(), d)); }
    
    // access to slack variables
    int  number_of_slack_variables( ) const { return slack_A.size(); }

    // access to artificial variables
    int  number_of_artificial_variables( ) const { return art_A.size(); }
    
    C_auxiliary_iterator
    c_auxiliary_value_iterator_begin( ) const { return aux_c.begin(); }
    C_auxiliary_iterator
    c_auxiliary_value_iterator_end( ) const {return aux_c.end(); }

    // access to basic variables
    int  number_of_basic_variables( ) const { return B_O.size()+B_S.size(); }
    int  number_of_basic_original_variables( ) const { return B_O.size(); }
    int  number_of_basic_slack_variables( ) const { return B_S.size(); }

    Basic_variable_index_iterator
    basic_original_variables_index_begin( ) const { return B_O.begin(); }
    Basic_variable_index_iterator
    basic_original_variables_index_end  ( ) const { return B_O.end(); }
    
    Basic_variable_numerator_iterator
    basic_original_variables_numerator_begin( ) const { return x_B_O.begin(); }
    Basic_variable_numerator_iterator
    basic_original_variables_numerator_end  ( ) const { return x_B_O.begin()
							       + B_O.size(); }
    Basic_variable_value_iterator
    basic_original_variables_value_begin( ) const
        { return Basic_variable_value_iterator(
                     basic_original_variables_numerator_begin(),
                     Quotient_maker( Quotient_creator(), d)); }
    Basic_variable_value_iterator
    basic_original_variables_value_end  ( ) const
        { return Basic_variable_value_iterator(
                     basic_original_variables_numerator_end(),
                     Quotient_maker( Quotient_creator(), d)); }

    Basic_variable_index_iterator
    basic_slack_variables_index_begin( ) const { return B_S.begin(); }
    Basic_variable_index_iterator
    basic_slack_variables_index_end  ( ) const { return B_S.end(); }
    
    Basic_variable_numerator_iterator
    basic_slack_variables_numerator_begin( ) const { return x_B_S.begin(); }
    Basic_variable_numerator_iterator
    basic_slack_variables_numerator_end  ( ) const { return x_B_S.begin()
							    + B_S.size(); }
    Basic_variable_value_iterator
    basic_slack_variables_value_begin( ) const
        { return Basic_variable_value_iterator(
                     basic_slack_variables_numerator_begin(),
                     Quotient_maker( Quotient_creator(), d)); }
    Basic_variable_value_iterator
    basic_slack_variables_value_end  ( ) const
        { return Basic_variable_value_iterator(
                     basic_slack_variables_numerator_end(),
                     Quotient_maker( Quotient_creator(), d)); }

    // access to working variables
    int  number_of_working_variables( ) const { return in_B.size(); }

    bool  is_basic( int j) const
        { CGAL_qpe_precondition( j >= 0);
          CGAL_qpe_precondition( j < number_of_working_variables());
          return ( in_B[ j] >= 0); }
	  
    bool is_artificial(int k) const;
    
    int get_l() const;

    // access to lambda
    Lambda_numerator_iterator
    lambda_numerator_begin( ) const { return lambda.begin(); }
    
    Lambda_numerator_iterator
    lambda_numerator_end  ( ) const { return lambda.begin() + C.size(); }

    Lambda_value_iterator
    lambda_value_begin( ) const
        { return Lambda_value_iterator(
                     lambda_numerator_begin(),
                     Quotient_maker( Quotient_creator(), d)); }
    
    Lambda_value_iterator
    lambda_value_end  ( ) const
        { return Lambda_value_iterator(
                     lambda_numerator_end(),
                     Quotient_maker( Quotient_creator(), d)); }

    // miscellaneous
    // -------------
    // altering the pricing strategy
    void  set_pricing_strategy( Pricing_strategy& strategy);

    // diagnostic output
    void  set_verbosity( int verbose = 0, std::ostream& stream = std::cout);

    // access to indices of basic constraints
    int  number_of_basic_constraints( ) const { return C.size(); }

    Basic_constraint_index_iterator
    basic_constraints_index_begin( ) const { return C.begin(); }
    Basic_constraint_index_iterator
    basic_constraints_index_end  ( ) const { return C.end(); }

    // helper functions
    template < class RndAccIt1, class RndAccIt2, class NT >  
    NT  mu_j_( int j, RndAccIt1 lambda_it, RndAccIt2 x_it, const NT& dd) const;

    ET  dual_variable( int i)
    {
	for ( int j = 0; j < qp_m; ++j) {
	    tmp_x[ j] = inv_M_B.entry( j, i);
	}
	return std::inner_product( tmp_x.begin(), tmp_x.begin()+qp_m,
				   minus_c_B.begin(), et0);
    }

  private:    

    // private member functions
    // ------------------------
    // initialization
    void  init_basis( );
    void  init_basis__slack_variables( int s_i, Tag_true  has_no_inequalities);
    void  init_basis__slack_variables( int s_i, Tag_false has_no_inequalities);
    void  init_basis__constraints    ( int s_i, Tag_true  has_no_inequalities);
    void  init_basis__constraints    ( int s_i, Tag_false has_no_inequalities);

    void  init_solution( );
    void  init_solution__b_C( Tag_true  has_no_inequalities);
    void  init_solution__b_C( Tag_false has_no_inequalities);

    void  init_additional_data_members( );
    int  signed_leading_exponent( int row);
    void  set_up_auxiliary_problemI( Tag_true is_perturbed);
    void  set_up_auxiliary_problem( Tag_true is_perturbed);
    void  set_up_auxiliary_problem( Tag_false is_perturbed);

    // transition (to phase II)
    void  transition( );
    void  transition( Tag_true  is_linear);
    void  transition( Tag_false is_linear);
    
    // pivot step
    void  pivot_step( );

    // pricing
    void  pricing( );

    template < class NT, class It >
    void  mu_j__linear_part_( NT& mu_j, int j, It lambda_it,
			      Tag_true  has_no_inequalities) const;
    template < class NT, class It >
    void  mu_j__linear_part_( NT& mu_j, int j, It lambda_it,
			      Tag_false has_no_inequalities) const;

    template < class NT, class It >
    void  mu_j__quadratic_part_( NT& mu_j, int j, It x_it,
				 Tag_true  is_linear) const;
    template < class NT, class It >
    void  mu_j__quadratic_part_( NT& mu_j, int j, It x_it,
				 Tag_false is_linear) const;
    template < class NT, class It >
    void  mu_j__quadratic_part_( NT& mu_j, int j, It x_it,
				 Tag_false is_linear,
				 Tag_true  is_symmetric) const;
    template < class NT, class It >
    void  mu_j__quadratic_part_( NT& mu_j, int j, It x_it,
				 Tag_false is_linear,
				 Tag_false is_symmetric) const;

    template < class NT, class It >
    void  mu_j__slack_or_artificial_( NT& mu_j, int j, It lambda_it, const NT& dd,
				      Tag_true  has_no_inequalities) const;
    template < class NT, class It >
    void  mu_j__slack_or_artificial_( NT& mu_j, int j, It lambda_it, const NT& dd,
				      Tag_false has_no_inequalities) const;

    // ratio test
    void  ratio_test_init( );
    void  ratio_test_init__A_Cj( Value_iterator A_Cj_it, int j,
				 Tag_true  has_no_inequalities);
    void  ratio_test_init__A_Cj( Value_iterator A_Cj_it, int j,
				 Tag_false has_no_inequalities);
    void  ratio_test_init__2_D_Bj( Value_iterator two_D_Bj_it, int j,
				   Tag_true  is_linear);
    void  ratio_test_init__2_D_Bj( Value_iterator two_D_Bj_it, int j,
				   Tag_false is_linear);
    void  ratio_test_init__2_D_Bj( Value_iterator two_D_Bj_it, int j,
				   Tag_false is_linear,
				   Tag_true  has_no_inequalities);
    void  ratio_test_init__2_D_Bj( Value_iterator two_D_Bj_it, int j,
				   Tag_false is_linear,
				   Tag_false has_no_inequalities);

    void  ratio_test_1( );
    void  ratio_test_1__q_x_O( Tag_true  is_linear);
    void  ratio_test_1__q_x_O( Tag_false is_linear);
    void  ratio_test_1__q_x_S( Tag_true  has_no_inequalities);
    void  ratio_test_1__q_x_S( Tag_false has_no_inequalities);
    void  ratio_test_1__t_i( Index_iterator i_it, Index_iterator end_it,
			     Value_iterator x_it, Value_iterator   q_it,
			     Tag_true  no_check);
    void  ratio_test_1__t_i( Index_iterator i_it, Index_iterator end_it,
			     Value_iterator x_it, Value_iterator   q_it,
			     Tag_false  no_check);
    void  ratio_test_1__t_j( Tag_true  is_linear);
    void  ratio_test_1__t_j( Tag_false is_linear);

    void  ratio_test_2( Tag_true  is_linear);
    void  ratio_test_2( Tag_false is_linear);
    void  ratio_test_2__p( Tag_true  has_no_inequalities);
    void  ratio_test_2__p( Tag_false has_no_inequalities);                    

    // update
    void  update_1( );
    void  update_1( Tag_true  is_linear);
    void  update_1( Tag_false is_linear);

    void  update_2( Tag_true  is_linear);
    void  update_2( Tag_false is_linear);

    void  replace_variable( );
    void  replace_variable( Tag_true  is_linear);
    void  replace_variable( Tag_false is_linear);
    void  replace_variable_original_original( );
    void  replace_variable_original_slack( );
    void  replace_variable_slack_original( );
    void  replace_variable_slack_slack( );
    void  remove_artificial_variable_and_constraint( );
    void  expel_artificial_variables_from_basis( );

    void  enter_variable( );
    void  leave_variable( );
    void  z_replace_variable( );
    void  z_replace_variable( Tag_true is_linear);
    void  z_replace_variable( Tag_false is_linear);
    void  z_replace_variable_original_by_original( );
    void  z_replace_variable_original_by_slack( );
    void  z_replace_variable_slack_by_original( );
    void  z_replace_variable_slack_by_slack( );

    bool  basis_matrix_stays_regular( );

    // current solution
    void  compute_solution( );

    void  compute__x_B_S( Tag_true  has_no_inequalities);
    void  compute__x_B_S( Tag_false has_no_inequalities);
    void  multiply__A_S_BxB_O( Value_iterator in, Value_iterator out) const;

    // check basis inverse
    bool  check_basis_inverse( );
    bool  check_basis_inverse( Tag_true  is_linear);
    bool  check_basis_inverse( Tag_false is_linear);

    // diagnostic output
    void  print_program ( );
    void  print_basis   ( );
    void  print_solution( );

    const char*  variable_type( int k) const;
    
    // ensure container size
    template <class Container>
    void ensure_size(Container& c, typename Container::size_type desired_size) {
    	typedef typename Container::value_type Value_type;
    	for (typename Container::size_type i=c.size(); i < desired_size; ++i) {
	    c.push_back(Value_type());
	}
    }
    
    // 
    public:
    bool is_solution_feasible();
    bool is_solution_feasible_aux();
    bool is_solution_optimal(Tag_false is_linear);
    bool is_solution_optimal(Tag_true is_linear);
    bool is_solution_optimal_aux();
    bool is_solution_valid();
    bool is_solution_unbounded(Tag_false is_linear);
    bool is_solution_unbounded(Tag_true is_linear);

// ----------------------------------------------------------------------------

// ===============================
// class implementation (template)
// ===============================

  public:

    // pricing
    // -------
    template < class RndAccIt1, class RndAccIt2, class NT >  
    NT
    mu_j( int j, RndAccIt1 lambda_it, RndAccIt2 x_it, const NT& dd) const
    {
	NT  mu_j;

	if ( j < qp_n) {                                // original variable

	    // [c_j +] A_Cj^T * lambda_C
	    mu_j = ( is_phaseI ? NT( 0) : dd * qp_c[ j]);
	    mu_j__linear_part( mu_j, j, lambda_it, Has_no_inequalities());

	    // ... + 2 D_Bj^T * x_B
	    mu_j__quadratic_part( mu_j, j, x_it, Is_linear());

	} else {                                        // slack or artificial

	    mu_j__slack_or_artificial( mu_j, j, lambda_it, dd,
				       Has_no_inequalities());

	}

	return mu_j;
    }

  private:

    // pricing (private helper functions)
    // ----------------------------------
    template < class NT, class It > inline                      // no ineq.
    void
    mu_j__linear_part( NT& mu_j, int j, It lambda_it, Tag_true) const
    {
	mu_j += inv_M_B.inner_product_l( lambda_it, qp_A[ j]);
    }

    template < class NT, class It > inline                      // has ineq.
    void
    mu_j__linear_part( NT& mu_j, int j, It lambda_it, Tag_false) const
    {
	mu_j += inv_M_B.inner_product_l( lambda_it,
					 A_by_index_iterator( C.begin(),
					     A_by_index_accessor( qp_A[ j])));
    }

    template < class NT, class It > inline                      // LP case
    void
    mu_j__quadratic_part( NT&, int, It, Tag_true) const
    {
	// nop
    }

    template < class NT, class It > inline                      // QP case
    void
    mu_j__quadratic_part( NT& mu_j, int j, It x_it, Tag_false) const
    {
	if ( is_phaseII) {
	    mu_j__quadratic_part( mu_j, j, x_it, Tag_false(), Is_symmetric());
	}
    }

    template < class NT, class It >  inline                     // QP, D sym.
    void
    mu_j__quadratic_part( NT& mu_j, int j, It x_it, Tag_false, Tag_true) const
    {
	// 2 D_Bj^T * x_B
	mu_j += inv_M_B.inner_product_x( x_it,
					 D_by_index_iterator( B_O.begin(),
					     D_by_index_accessor( qp_D[ j])))
	        * NT( 2);
    }

    template < class NT, class It >  inline                     // QP, D no-sym
    void
    mu_j__quadratic_part( NT& mu_j, int j, It x_it, Tag_false, Tag_false) const
    {
	// ( D_Bj^T + D_jB) * x_B
	mu_j += inv_M_B.inner_product_x( x_it,
					 D_pairwise_iterator( B_O.begin(),
					     D_pairwise_accessor( qp_D, j)));
    }

    template < class NT, class It >  inline                     // no ineq.
    void
    mu_j__slack_or_artificial( NT& mu_j, int j, It lambda_it, const NT& dd, Tag_true) const
    {
	j -= qp_n;
                                                        // artificial variable
	// A_j^T * lambda
	mu_j = lambda_it[ j];
	if ( art_A[ j].second) mu_j = -mu_j;

	// c_j + ...
	mu_j += dd*aux_c[ j];

    }

    template < class NT, class It >  inline                     // has ineq.
    void
    mu_j__slack_or_artificial( NT& mu_j, int j, It lambda_it, const NT& dd, Tag_false) const
    {
	j -= qp_n;

	if ( j < (int)slack_A.size()) {                 // slack variable

	    // A_Cj^T * lambda_C
	    mu_j = lambda_it[ in_C[ slack_A[ j].first]];
	    if ( slack_A[ j].second) mu_j = -mu_j;

	} else {                                        // artificial variable
	    j -= slack_A.size();

	    // A_Cj^T * lambda_C
	    mu_j = lambda_it[ in_C[ art_A[ j].first]];
	    if ( art_A[ j].second) mu_j = -mu_j;

	    // c_j + ...
	    mu_j += dd*aux_c[ j];
	}
    }

};

// ----------------------------------------------------------------------------

// =============================
// class implementation (inline)
// =============================

// initialization
// --------------
template < class Rep_ >  inline                                 // no ineq.
void  QPE_solver<Rep_>::
init_basis__slack_variables( int, Tag_true)
{
    // nop
}

template < class Rep_ >  inline                                 // no ineq.
void  QPE_solver<Rep_>::
init_basis__constraints( int, Tag_true)
{
    // create 'm' dummy entries in 'C'
    C.reserve( qp_m);
    for ( i = 0; i < qp_m; ++i) C.push_back( i);
}

template < class Rep_ >  inline                                 // no ineq.
void  QPE_solver<Rep_>::
init_solution__b_C( Tag_true)
{
    b_C.reserve( qp_m);
    std::copy( qp_b, qp_b+qp_m, std::back_inserter( b_C));
}

template < class Rep_ >  inline                                 // has ineq.
void  QPE_solver<Rep_>::
init_solution__b_C( Tag_false)
{ 
    b_C.insert( b_C.end(), l, et0);
    B_by_index_accessor  b_accessor( qp_b);
    std::copy( B_by_index_iterator( C.begin(), b_accessor),
	       B_by_index_iterator( C.end  (), b_accessor),
	       b_C.begin());
}

// transition
// ----------
template < class Rep_ >  inline                                 // QP case
void  QPE_solver<Rep_>::
transition( Tag_false)
{
    typedef  Creator_2< D_iterator, int, 
	         D_pairwise_accessor >  D_transition_creator_accessor;

    typedef  Creator_2< Index_iterator, D_pairwise_accessor,
	         D_pairwise_iterator >  D_transition_creator_iterator;

    typedef  QPE_transform_iterator_1< Index_iterator, typename Bind<
	         typename Compose< D_transition_creator_iterator,
	             Identity< Index_iterator >, typename
	             Bind< D_transition_creator_accessor, D_iterator, 1 >::Type >::Type,
                            Index_iterator, 1>::Type >
	                                twice_D_transition_iterator;

    inv_M_B.transition( twice_D_transition_iterator( B_O.begin(),
	bind_1( compose( D_transition_creator_iterator(),
	    Identity<Index_iterator>(), bind_1(
		D_transition_creator_accessor(), qp_D)), B_O.begin())));
}

template < class Rep_ >  inline                                 // LP case
void  QPE_solver<Rep_>::
transition( Tag_true)
{
    inv_M_B.transition();
}

// ratio test
// ----------
template < class Rep_ > inline                                  // LP case
void  QPE_solver<Rep_>::
ratio_test_init__2_D_Bj( Value_iterator, int, Tag_true)
{
    // nop
}

template < class Rep_ > inline                                  // QP case
void  QPE_solver<Rep_>::
ratio_test_init__2_D_Bj( Value_iterator two_D_Bj_it, int j_, Tag_false)
{
    if ( is_phaseII) {
	ratio_test_init__2_D_Bj( two_D_Bj_it, j_,
				 Tag_false(), Has_no_inequalities());
    }
}

template < class Rep_ > inline                                  // QP, no ineq.
void  QPE_solver<Rep_>::
ratio_test_init__2_D_Bj( Value_iterator two_D_Bj_it, int j_, Tag_false,
			                                     Tag_true )
{
    // store exact version of `2 D_{B_O,j}'
    D_pairwise_accessor  d_accessor( qp_D, j_);
    std::copy( D_pairwise_iterator( B_O.begin(), d_accessor),
	       D_pairwise_iterator( B_O.end  (), d_accessor),
	       two_D_Bj_it);
}

template < class Rep_ > inline                                  // QP, has ineq
void  QPE_solver<Rep_>::
ratio_test_init__2_D_Bj( Value_iterator two_D_Bj_it, int j_, Tag_false,
			                                     Tag_false)
{
    // store exact version of `2 D_{B_O,j}'
    if ( j_ < qp_n) {                               // original variable
	ratio_test_init__2_D_Bj( two_D_Bj_it, j_, Tag_false(), Tag_true());
    } else {                                        // slack variable
	std::fill_n( two_D_Bj_it, B_O.size(), et0);
    }
}

template < class Rep_ >  inline                                 // LP case
void  QPE_solver<Rep_>::
ratio_test_1__q_x_O( Tag_true)
{
    inv_M_B.multiply_x( A_Cj.begin(), q_x_O.begin());
}

template < class Rep_ >  inline                                 // QP case
void  QPE_solver<Rep_>::
ratio_test_1__q_x_O( Tag_false)
{
    if ( is_phaseI) {                                   // phase I
	inv_M_B.multiply_x(     A_Cj.begin(),    q_x_O.begin());
    } else {                                            // phase II
	inv_M_B.multiply  (     A_Cj.begin(), two_D_Bj.begin(),
			    q_lambda.begin(),    q_x_O.begin());
    }
}

template < class Rep_ >  inline                                 // no ineq.
void  QPE_solver<Rep_>::
ratio_test_1__q_x_S( Tag_true)
{
    // nop
}

template < class Rep_ >  inline                                 // has ineq.
void  QPE_solver<Rep_>::
ratio_test_1__q_x_S( Tag_false)
{
    // A_S_BxB_O * q_x_O
    multiply__A_S_BxB_O( q_x_O.begin(), q_x_S.begin());

    // ( A_S_BxB_O * q_x_O) - A_S_Bxj
    if ( j < qp_n) {
	std::transform( q_x_S.begin(),
			q_x_S.begin()+S_B.size(),
			A_by_index_iterator( S_B.begin(),
					     A_by_index_accessor( qp_A[ j])),
			q_x_S.begin(),
			compose2_2( std::minus<ET>(),
				    Identity<ET>(),
				    std::bind1st( std::multiplies<ET>(), d)));
    }

    // q_x_S = -+ ( A_S_BxB_O * q_x_O - A_S_Bxj)
    Value_iterator  q_it = q_x_S.begin();
    Index_iterator  i_it;
    for ( i_it = B_S.begin(); i_it != B_S.end(); ++i_it, ++q_it) {
	if ( ! slack_A[ *i_it - qp_n].second) *q_it = -(*q_it);
    }
}

template < class Rep_ >  inline                                 // no check
void  QPE_solver<Rep_>::
ratio_test_1__t_i( Index_iterator, Index_iterator,
		   Value_iterator, Value_iterator, Tag_true)
{
    // nop
}

template < class Rep_ >  inline                                 // check
void  QPE_solver<Rep_>::
ratio_test_1__t_i( Index_iterator i_it, Index_iterator end_it,
		   Value_iterator x_it, Value_iterator   q_it, Tag_false)
{
    // check `t_i's
    for ( ; i_it != end_it; ++i_it, ++x_it, ++q_it) {
	if ( ( *q_it > et0) && ( ( *x_it * q_i) < ( x_i * *q_it))) {
	    i = *i_it; x_i = *x_it; q_i = *q_it;
	}
    }
}

template < class Rep_ >  inline                                 // LP case
void  QPE_solver<Rep_>::
ratio_test_1__t_j( Tag_true)
{
    // nop
}

template < class Rep_ >  inline                                 // QP case
void  QPE_solver<Rep_>::
ratio_test_1__t_j( Tag_false)
{
    if ( is_phaseII) {

	// compute `nu' and `mu_j' 
	mu = inv_M_B.inner_product(     A_Cj.begin(), two_D_Bj.begin(),
				      lambda.begin(),    x_B_O.begin());
	nu = inv_M_B.inner_product(     A_Cj.begin(), two_D_Bj.begin(),
				    q_lambda.begin(),    q_x_O.begin());
	if ( j < qp_n) {                                // original variable
	    mu +=     d*ET( qp_c[ j]);
	    nu -= et2*d*ET( qp_D[ j][ j]);
	}
	CGAL_qpe_assertion(nu <= et0);

	// check `t_j'
	if ( ( nu < et0) && ( ( mu * q_i) > ( x_i * nu))) {
	    i = -1; q_i = et1;
	}
    }
}

template < class Rep_ >  inline                                 // LP case
void  QPE_solver<Rep_>::
ratio_test_2( Tag_true)
{
    // nop
}

template < class Rep_ >  inline                                 // no ineq.
void  QPE_solver<Rep_>::
ratio_test_2__p( Tag_true)
{
    // get column index of entering variable in basis
    int  col = in_B[ j];
 
    CGAL_qpe_assertion( col >= 0);
    col += l;

    // get (last) column of `M_B^{-1}' (Note: `p_...' is stored in `q_...')
    Value_iterator  it;
    int             row;
    unsigned int    k;
    for (   k = 0,            row = 0,   it = q_lambda.begin();
	    k < C.size();
	  ++k,              ++row,     ++it                   ) {
	*it = inv_M_B.entry( row, col);
    }
    for (   k = 0,            row = l,   it = q_x_O.begin();
	    k < B_O.size();
	  ++k,              ++row,     ++it                   ) {
	*it = inv_M_B.entry( row, col);
    }
}

template < class Rep_ >  inline                                 // has ineq.
void  QPE_solver<Rep_>::
ratio_test_2__p( Tag_false)
{
    Value_iterator  v_it;
    Index_iterator  i_it;

    // compute 'p_lambda' and 'p_x_O' (Note: `p_...' is stored in `q_...')
    // -------------------------------------------------------------------

    // type of entering variable
    if ( j < qp_n) {                                        // original

	// use 'no_ineq' variant
	ratio_test_2__p( Tag_true());

    } else {                                                // slack

	j -= qp_n;

	// get column A_{S_j,B_O}^T (i.e. row of A_{S_B,B_O})
	int             row  = slack_A[ j].first;
	bool            sign = slack_A[ j].second;

	for (   i_it =  B_O.begin(),   v_it = tmp_x.begin();
	        i_it != B_O.end();
	      ++i_it,                ++v_it                ) {
	    *v_it = ( sign ? qp_A[ *i_it][ row] : -qp_A[ *i_it][ row]);
	}

	// compute  ( p_l | p_x_O )^T = M_B^{-1} * ( 0 | A_{S_j,B_O} )^T
	std::fill_n( tmp_l.begin(), C.size(), et0);
	inv_M_B.multiply( tmp_l     .begin(), tmp_x  .begin(),
			  q_lambda.begin(),   q_x_O.begin());

	j += qp_n;
    }

    // compute 'p_x_S'
    // ---------------
    // A_S_BxB_O * p_x_O
    multiply__A_S_BxB_O( q_x_O.begin(), q_x_S.begin());

    // p_x_S = +- ( A_S_BxB_O * p_x_O)
    for (   i_it =  B_S.begin(),   v_it = q_x_S.begin();
	    i_it != B_S.end();
	  ++i_it,                ++v_it                ) {
	if ( ! slack_A[ *i_it - qp_n].second) *v_it = -(*v_it);
    }
}

// update
// ------
template < class Rep_ >  inline                                 // LP case
void  QPE_solver<Rep_>::
update_1( Tag_true)
{
    // replace leaving with entering variable
    replace_variable();
}

template < class Rep_ >  inline                                 // QP case
void  QPE_solver<Rep_>::
update_1( Tag_false)
{
    if ( is_phaseI) {                                   // phase I

	// replace leaving with entering variable
	replace_variable();

    } else {                                            // phase II

	if ( ( i >= 0) && basis_matrix_stays_regular()) {

	    // leave variable from basis, if
	    // - some leaving variable was found  and
	    // - basis matrix stays regular
	    leave_variable();

	} else {

	    // enter variable into basis, if
	    // - no leaving variable was found  or
	    // - basis matrix would become singular when variable i leaves

	    if ( i < 0 ) {
	        enter_variable();
	    } else {
	        z_replace_variable();
	    }
	}
    }
}

template < class Rep_ >  inline                                 // LP case
void  QPE_solver<Rep_>::
update_2( Tag_true)
{
    // nop
}

template < class Rep_ >  inline                                 // no ineq.
void  QPE_solver<Rep_>::
replace_variable( Tag_true)
{
    replace_variable_original_original();
    strategyP->leaving_basis( i);
}

template < class Rep_ >  inline                                 // has ineq.
void  QPE_solver<Rep_>::
replace_variable( Tag_false)
{
    // determine type of variables
    bool  enter_original = ( (j < qp_n) || (j >= (int)( qp_n+slack_A.size())));
    bool  leave_original = ( (i < qp_n) || (i >= (int)( qp_n+slack_A.size())));


    // update basis & basis inverse
    if ( leave_original) {
	if ( enter_original) {                              // orig  <--> orig
	    replace_variable_original_original();
	} else {                                            // slack <--> orig
	    replace_variable_slack_original();
	}

	// special artificial variable removed?
	if ( is_phaseI && ( i == art_s_i)) {
	    // remove the fake column - it corresponds
	    // to the special artificial variable which is
	    // (like all artificial variables) not needed
	    // anymore once it leaves the basis. Note:
	    // regular artificial variables are only removed
	    // from the problem after phase I
	    // art_s_i == -1 -> there is no special artificial variable
	    // art_s_i == -2 -> there was a special artificial variable, 
	    // but has been removed  
	    art_s_i = -2;
	    art_A.pop_back();
	    in_B.pop_back();
	} else {
	    strategyP->leaving_basis( i);
	}
    } else {
	if ( enter_original) {                              // orig  <--> slack
	    replace_variable_original_slack();
	} else {                                            // slack <--> slack
	    replace_variable_slack_slack();
	}
	strategyP->leaving_basis( i);
    }
}

template < class Rep_ >  inline
bool  QPE_solver<Rep_>::
basis_matrix_stays_regular()
{
    CGAL_qpe_precondition( is_phaseII);
    int new_row, k;
    
    if ( has_ineq && (i >= qp_n)) {	// slack variable
	new_row = slack_A[ i-qp_n].first;
	A_row_by_index_accessor  a_accessor( A_accessor( qp_A, 0, qp_n), new_row);
	std::copy( A_row_by_index_iterator( B_O.begin(), a_accessor),
		   A_row_by_index_iterator( B_O.end  (), a_accessor),
		   tmp_x.begin());	   
	inv_M_B.multiply( tmp_x.begin(),                        // dummy (not used)
		  tmp_x.begin(), tmp_l_2.begin(), tmp_x_2.begin(),
		  Tag_false(),                                 // QP
		  Tag_false());                             // ignore 1st argument
        return ( -inv_M_B.inner_product_x( tmp_x_2.begin(), tmp_x.begin()) != et0);

	
    } else {						// check original variable
    	k = l+in_B[ i];
    	return ( inv_M_B.entry( k, k) != et0);
    }

/* ToDo: check, if really not needed in 'update_1':
  - basis has already minimal size  or
  || ( B_O.size()==C.size()) 
*/
}

// current solution
// ----------------
template < class Rep_ >  inline                                 // no ineq.
void  QPE_solver<Rep_>::
compute__x_B_S( Tag_true)
{
    // nop
}

template < class Rep_ >  inline                                // has ineq.
void  QPE_solver<Rep_>::
compute__x_B_S( Tag_false)
{
    // A_S_BxB_O * x_B_O
    multiply__A_S_BxB_O( x_B_O.begin(), x_B_S.begin());

    // b_S_B - ( A_S_BxB_O * x_B_O)
    B_by_index_accessor  b_accessor( qp_b);
    std::transform( B_by_index_iterator( S_B.begin(), b_accessor),
		    B_by_index_iterator( S_B.end  (), b_accessor),
		    x_B_S.begin(),
		    x_B_S.begin(),
		    compose2_2( std::minus<ET>(),
				std::bind1st( std::multiplies<ET>(), d),
				Identity<ET>()));

    // x_B_S = +- ( b_S_B - A_S_BxB_O * x_B_O)
    Value_iterator  x_it = x_B_S.begin();
    Index_iterator  i_it;
    for ( i_it = B_S.begin(); i_it != B_S.end(); ++i_it, ++x_it) {
	if ( slack_A[ *i_it - qp_n].second) *x_it = -(*x_it);
    }
       
}

CGAL_END_NAMESPACE


#ifdef CGAL_CFG_NO_AUTOMATIC_TEMPLATE_INCLUSION
#  include <CGAL/QPE_solver.C>
#endif

#endif // CGAL_QPE_SOLVER_H

// ===== EOF ==================================================================
