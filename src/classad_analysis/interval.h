/***************************Copyright-DO-NOT-REMOVE-THIS-LINE**
  *
  * Condor Software Copyright Notice
  * Copyright (C) 1990-2006, Condor Team, Computer Sciences Department,
  * University of Wisconsin-Madison, WI.
  *
  * This source code is covered by the Condor Public License, which can
  * be found in the accompanying LICENSE.TXT file, or online at
  * www.condorproject.org.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  * AND THE UNIVERSITY OF WISCONSIN-MADISON "AS IS" AND ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  * WARRANTIES OF MERCHANTABILITY, OF SATISFACTORY QUALITY, AND FITNESS
  * FOR A PARTICULAR PURPOSE OR USE ARE DISCLAIMED. THE COPYRIGHT
  * HOLDERS AND CONTRIBUTORS AND THE UNIVERSITY OF WISCONSIN-MADISON
  * MAKE NO MAKE NO REPRESENTATION THAT THE SOFTWARE, MODIFICATIONS,
  * ENHANCEMENTS OR DERIVATIVE WORKS THEREOF, WILL NOT INFRINGE ANY
  * PATENT, COPYRIGHT, TRADEMARK, TRADE SECRET OR OTHER PROPRIETARY
  * RIGHT.
  *
  ****************************Copyright-DO-NOT-REMOVE-THIS-LINE**/

#ifndef __INTERVAL_H__
#define __INTERVAL_H__

#define WANT_NAMESPACES
#include "classad_distribution.h"
#include "list.h"
#include "extArray.h"


struct Interval
{
	Interval() { key = -1; openLower = openUpper = false; }
	int				key;
	classad::Value 	lower, upper;
	bool			openLower, openUpper;
};

bool Copy( Interval *src, Interval *dest );

bool GetLowValue ( Interval *, classad::Value &result );
bool GetHighValue ( Interval *, classad::Value &result );
bool GetLowDoubleValue ( Interval *, double & );
bool GetHighDoubleValue ( Interval *, double & );

bool Overlaps( Interval *, Interval * );
bool Precedes( Interval *, Interval * );
bool Consecutive( Interval *, Interval * );
bool StartsBefore( Interval *, Interval * );
bool EndsAfter( Interval *, Interval * );

classad::Value::ValueType GetValueType( Interval * );
bool IntervalToString( Interval *, std::string & );

bool Numeric( classad::Value::ValueType );
bool SameType( classad::Value::ValueType vt1, classad::Value::ValueType vt2 );
bool GetDoubleValue ( classad::Value &, double & );
bool EqualValue( classad::Value &, classad::Value & );
bool IncrementValue( classad::Value & );
bool DecrementValue( classad::Value & );


class HyperRect;

class IndexSet
{
 public:
	IndexSet( );
	~IndexSet( );
	bool Init( int _size );
	bool Init( IndexSet & );
	bool AddIndex( int );
	bool RemoveIndex( int );
	bool RemoveAllIndeces( );
	bool AddAllIndeces( );
	bool GetCardinality( int & );
	bool Equals( IndexSet & );
	bool IsEmpty( );
	bool HasIndex( int );
	bool ToString( std::string &buffer );
	bool Union( IndexSet & );
	bool Intersect( IndexSet & );

	static bool Translate( IndexSet &, int *map, int mapSize, int newSize, 
						   IndexSet &result );
	static bool Union( IndexSet &, IndexSet &, IndexSet &result );
	static bool Intersect( IndexSet &, IndexSet &, IndexSet &result );

 private:
	bool initialized;
	int size;
	int cardinality;
	bool *inSet;
};

struct MultiIndexedInterval
{
	MultiIndexedInterval( ) { ival = NULL; }
	Interval *ival;
	IndexSet iSet;
};

class ValueRange
{
 public:
	ValueRange( );
	~ValueRange( );
	bool Init( Interval *, bool undef=false, bool notString=false );
	bool Init2( Interval *, Interval *, bool undef=false );
	bool InitUndef( );
	bool Init( ValueRange *, int index, int numIndeces );
	bool Intersect( Interval *, bool undef=false, bool notString=false );
	bool Intersect2( Interval *, Interval *, bool undef=false );
	bool IntersectUndef( );
	bool Union( ValueRange *, int index );
	bool IsEmpty( );
	bool EmptyOut( );
	bool GetDistance( classad::Value &pt, classad::Value &min,
					  classad::Value &max, double &result,
					  classad::Value &nearestVal );

	static bool BuildHyperRects( ExtArray< ValueRange * > &, int dimensions,
								 int numContexts, 
								 List< ExtArray< HyperRect * > > & );
   
	bool ToString( std::string & );
	bool IsInitialized( );
 private:
	bool initialized;
	classad::Value::ValueType type;
	bool multiIndexed;
	List< MultiIndexedInterval > miiList;
	int numIndeces;
	List< Interval > iList;
	bool anyOtherString;
	IndexSet anyOtherStringIS;
	bool undefined;
	IndexSet undefinedIS;
};


class ValueRangeTable
{
 public:
	ValueRangeTable( );
	~ValueRangeTable( );
	bool Init( int numCols, int numRows );
	bool SetValueRange( int col, int row, ValueRange * );
	bool GetValueRange( int col, int row, ValueRange *& );
	bool GetNumRows( int & );
	bool GetNumColumns( int & );
	bool ToString( std::string &buffer );
 private:
	bool initialized;
	int numCols;
	int numRows;
	ValueRange ***table;
};

class ValueTable
{
 public:
	ValueTable( );
	~ValueTable( );
	bool Init( int numCols, int numRows );
	bool SetOp( int row, classad::Operation::OpKind );
	bool SetValue( int col, int row, classad::Value & );
	bool GetValue( int col, int row, classad::Value & );
	bool GetNumRows( int & );
	bool GetNumColumns( int & );
	bool GetUpperBound( int row, classad::Value & );
	bool GetLowerBound( int row, classad::Value & );
	bool ToString( std::string &buffer );
 private:
	bool initialized;
	int numCols;
	int numRows;
	bool inequality;
	classad::Value ***table;
	Interval **bounds;
	static bool IsInequality( classad::Operation::OpKind );
	static bool OpToString( std::string &buffer, classad::Operation::OpKind );
};

class HyperRect
{
 public:
	HyperRect( );
	~HyperRect( );
	bool Init( int dimensions, int numContexts );
	bool Init( int dimensions, int numContexts, Interval **& );
	bool GetInterval( int dim, Interval *& );
	bool SetIndexSet( IndexSet & );
	bool GetIndexSet( IndexSet & );
	bool FillIndexSet( );
	bool GetDimensions( int & );
	bool GetNumContexts( int & );
	bool ToString( std::string &buffer );
    HyperRect& operator=(const HyperRect & copy) { return *this; }
 private:
	bool initialized;
	int dimensions;
	int numContexts;
	IndexSet iSet;
	Interval **ivals;
};


#endif
