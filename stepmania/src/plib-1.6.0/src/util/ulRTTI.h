/*
  Originally written by: Alexandru C. Telea <alext@win.tue.nl>
*/

/*
  This file provides support for RTTI and generalized (virtual-base to derived
  and separate hierarchy branches) casting. There is also support for RT obj
  creation from type names.

  In order to enable these features for a class, two things should be done:

  1)	insert the text UL_TYPE_DATA (without ';') in the class-decl.
  2)	in the .C file where the class's implementation resides, insert the
	following (without';'):

	UL_RTTI_DEF(classname)
	if the class has no bases with RTTI

	UL_RTTI_DEFn(classname,b1,...bn)
	if the class has bases b1,...bn with RTTI

	Use UL_RTTI_DEF_INST instead of UL_RTTI_DEF if you want to enable RT
	obj creation for classname. You should provide then a public default
	ctor.

  RTTI is used via a class called ulRTTItypeid. A typeid describes a type of a
  class. [..] They should provide all necessary support for any kind of
  RTTI/casting [..].

  [..]

  REMARK:	There are two classes related to RTTI: ulRTTItypeid and
  =======	ulRTTITypeinfo. A ulRTTItypeid is, as it says, an 'id for a
		type'. It actually wraps a ulRTTITypeinfo*, where a
		ulRTTITypeinfo contains the actual encoding of a class type.
		You can freely create/copy/destroy/manipulate ulRTTItypeid's,
		but you should NEVER deal directly with ulRTTITypeinfo. A
		ulRTTITypeinfo should actually be created ONLY by the
		UL_TYPE_DATA macros, as part of a class definition, since the
		ulRTTITypeinfo encodes a type info for an EXISTING class [..].
		All type-related stuff should be therefore handled via
		ulRTTItypeid's.
*/

#ifndef _UL_RTTI_H_
#define _UL_RTTI_H_

#include <string.h>
#include "ul.h"


class ulRTTITypeinfo
/* Implementation of type-related info */
{
private:

  char *n ; /* type name */

  /*
    base types (NULL-ended array of ulRTTITypeinfo's for this's direct bases)
  */
  const ulRTTITypeinfo** b ;

  int ns ;  /* #subtypes of this type */
  const ulRTTITypeinfo **subtypes ; /* types derived from this type */

  /* convenience type info for a 'null' type */
  static const ulRTTITypeinfo null_type ;

  void* (*new_obj)() ;       /* func to create a new obj of this type */
  void* (*cast)(int,void*) ; /*
                               func to cast an obj of this type to ith
                               baseclass of it or to itself
                             */

  /* adds a subtype to this's subtypes[] */
  void add_subtype ( const ulRTTITypeinfo * ) ;

  /* dels a subtype from this's subtypes[] */
  void del_subtype ( const ulRTTITypeinfo* ) ;

  friend class ulRTTItypeid ; /* for null_type */

public:

  ulRTTITypeinfo ( const char* name, const ulRTTITypeinfo* bb[],
                   void* (*)(int,void*),void* (*)() ) ;
  ~ulRTTITypeinfo () ;

  /* Returns name of this ulRTTITypeinfo */
  const char* getname () const { return n ; }

  /* Compares 2 ulRTTITypeinfo objs */
  bool same ( const ulRTTITypeinfo *p ) const
  {
    /*
      First, try to see if it's the same 'physical' ulRTTITypeinfo (which
      should be the case, since we create them per-class and not per-obj).
    */
    return ( this == p ) || !strcmp ( n, p->n ) ;
  }

  /* true if the arg can be cast to this, else false */
  bool can_cast ( const ulRTTITypeinfo *p ) const
  {
    return same ( p ) || p->has_base ( this ) ;
  }

  /* true if this has the arg as some base, else false */
  bool has_base ( const ulRTTITypeinfo *p ) const
  {
    for ( int i = 0 ; b[i] != NULL ; i++ ) /* for all bases of this... */
      /* match found, return 1 or no match, search deeper */
      if ( p->same ( b[i] ) || b[i]->has_base ( p ) ) return true ;
    return false ; /* no match at all, return false */
  }

  /* get i-th subclass of this, if any, else NULL */
  const ulRTTITypeinfo * subclass ( int i = 0 ) const
  {
    return ( i >= 0 && i < ns ) ? subtypes[i] : NULL ;
  }

  int num_subclasses () const { return ns ; } /* get # subclasses of this */

  /*
    search for a subclass named char*, create obj of it and return it cast to
    the ulRTTITypeinfo* type, which is either this or a direct base of this.
  */
  void * create ( const ulRTTITypeinfo *, const char * ) const ;

  /* Returns true if this type has a default ctor, else false */
  bool can_create () const { return new_obj != NULL ; }
} ;


class ulRTTItypeid	
/* Main class for RTTI interface */
{
protected:

  /* ulRTTItypeid implementation (the only data-member) */
  const ulRTTITypeinfo* id ;

public:

  /* Not for application use ! */
  const ulRTTITypeinfo* get_info () const { return id ; }

  ulRTTItypeid ( const ulRTTITypeinfo* p ) : id ( p ) { }
  ulRTTItypeid () : id ( &ulRTTITypeinfo::null_type ) { }

  /* Compares 2 ulRTTItypeid objs */
  bool isSame ( ulRTTItypeid i ) const { return id->same ( i.id ) ; }

  /* true if the arg can be cast to this, else false */
  bool canCast ( ulRTTItypeid i ) const { return id->can_cast ( i.id ) ; }

  const char * getName () const { return id->getname () ; }

  /* Return # subclasses of this  */
  int getNumSubclasses  () const { return id->num_subclasses () ; }

  /* Return ith subclass of this */
  ulRTTItypeid getSubclass ( int i ) const { return id->subclass ( i ) ; }

  /* Return # baseclasses of this */
  int getNumBaseclasses () const
  {
    int i ; for ( i = 0 ; id->b[i] != NULL ; i++ ) ;
    return i ;
  }

  /* Return ith baseclass of this */
  ulRTTItypeid getBaseclass ( int i ) const { return id->b[i] ; }

  /*
    Tries to create an instance of a subclass of this having of type given
    by the ulRTTItypeid arg. If ok, it returns it casted to the class-type of
    this and then to void*
  */
  void * create ( ulRTTItypeid t ) const
  {
    return id->create ( id, t.getName () ) ; 
  }

  /* Returns true if this type is instantiable, else false */
  bool canCreate () const { return id->can_create () ; }
} ;


class ulRTTIdyntypeid : public ulRTTItypeid
/*
  Class for dynamic type creation from user strings. Useful for creating
  typeids at RT for comparison purposes.
*/
{
private:

  static const ulRTTITypeinfo *a[] ;

public:

  ulRTTIdyntypeid ( const char *c ) :
     /* create a dummy ulRTTITypeinfo */
     ulRTTItypeid ( new ulRTTITypeinfo ( c, a, NULL, NULL ) ) { }

  ~ulRTTIdyntypeid () { delete id ; /* delete the dummy ulRTTITypeinfo */ }
} ;



/*
  Macros
*/

/*
  'ulRTTItypeid'
  UL_STATIC_TYPE_INFO(T)	T=RTTI-class name.
				Returns a ulRTTItypeid with T's type. If T
				hasn't RTTI, a compile-time error occurs.
*/

#define UL_STATIC_TYPE_INFO(T) T::RTTI_sinfo()


/*
  'T*'
  UL_PTR_CAST(T,p)	T=RTTI-class, p=RTTI-class ptr.
			Returns p cast to the type T as a T*, if cast is
			possible, else returns NULL. If *p or T have no RTTI,
			a compile-time error occurs. Note that p can point to
			virtual base classes. Casting between separat branches
			of a class hierarchy is also supported, as long as all
			classes have RTTI. Therefore UL_PTR_CAST is a fully
			general and safe operator. If p==NULL, the operator
			returns NULL.
*/

#define UL_PTR_CAST(T,p)   ((p != NULL)? (T*)((p)->RTTI_cast(UL_STATIC_TYPE_INFO(T))) : NULL)


/* 'T*'
   UL_TYPE_NEW(T,t)	T=RTTI-class, t=ulRTTItypeid
			Returns a new object of type t cast to the type T as
			a T*. t must represent a type identical to or derived
			from T. If t is not a type derived from T or not an
                        instantiable type having a default constructor, NULL is
                        returned. */

#define UL_TYPE_NEW(T,t)   ((T*)t.create(T))



/*
  Definition of TYPE_DATA for a RTTI-class: introduces one static
  ulRTTITypeinfo data-member and a couple of virtuals.
*/

#define UL_TYPE_DATA			 		          \
	protected:					          \
	   static  const  ulRTTITypeinfo RTTI_obj; 		  \
	   static  void*  RTTI_scast(int,void*);	          \
	   static  void*  RTTI_new();			          \
	   virtual ulRTTItypeid RTTI_vinfo() const { return &RTTI_obj; }\
	public:						          \
	   static  ulRTTItypeid RTTI_sinfo()	 { return &RTTI_obj; }\
	   virtual void*  RTTI_cast(ulRTTItypeid);



/*
  Definition of auxiliary data-structs supporting RTTI for a class: defines
  the static ulRTTITypeinfo object of that class and its associated virtuals.
*/

/* Auxiliary definition of the construction method: */
#define UL_RTTI_NEW(cls)     void* cls::RTTI_new() { return new cls; }	\
			     const ulRTTITypeinfo cls::RTTI_obj(#cls,RTTI_base_ ## cls,cls::RTTI_scast,cls::RTTI_new);

#define UL_RTTI_NO_NEW(cls)  const ulRTTITypeinfo cls::RTTI_obj(#cls,RTTI_base_ ## cls,cls::RTTI_scast,NULL);



/*
  Top-level macros:
*/

#define UL_RTTI_DEF_BASE(cls)						\
	static const ulRTTITypeinfo* RTTI_base_ ## cls [] = { NULL };	\
	void* cls::RTTI_cast(ulRTTItypeid t)				\
	{								\
	   if (t.isSame(&RTTI_obj)) return this;			\
	   return NULL;							\
	}								\
	void* cls::RTTI_scast(int i,void* p)				\
	{  cls* ptr = (cls*)p; return ptr; }			
	

#define UL_RTTI_DEF1_BASE(cls,b1)					\
        static const ulRTTITypeinfo* RTTI_base_ ## cls [] = 		\
	       { UL_STATIC_TYPE_INFO(b1).get_info(), NULL };		\
  	void* cls::RTTI_cast(ulRTTItypeid t)				\
	{								\
	   if (t.isSame(&RTTI_obj)) return this;			\
	   void* ptr;							\
	   if ((ptr=b1::RTTI_cast(t))) return ptr;			\
	   return NULL;							\
	}								\
	void* cls::RTTI_scast(int i,void* p)				\
	{  cls* ptr = (cls*)p;						\
	   switch(i)							\
	   {  case  0: return (b1*)ptr;	 }				\
	   return ptr;							\
	}							
									

#define UL_RTTI_DEF2_BASE(cls,b1,b2)					\
        static const ulRTTITypeinfo* RTTI_base_ ## cls [] = 		\
	       { UL_STATIC_TYPE_INFO(b1).get_info(),			\
		 UL_STATIC_TYPE_INFO(b2).get_info(), NULL };		\
  	void* cls::RTTI_cast(ulRTTItypeid t)				\
	{								\
	   if (t.isSame(&RTTI_obj)) return this;			\
	   void* ptr;							\
	   if ((ptr=b1::RTTI_cast(t))) return ptr;			\
	   if ((ptr=b2::RTTI_cast(t))) return ptr;			\
	   return NULL;							\
	}								\
	void* cls::RTTI_scast(int i,void* p)				\
	{  cls* ptr = (cls*)p;						\
	   switch(i)							\
	   {  case  0: return (b1*)ptr;					\
	      case  1: return (b2*)ptr;					\
	   }								\
	   return ptr;							\
	}							
	
#define UL_RTTI_DEF3_BASE(cls,b1,b2,b3)					\
        static const ulRTTITypeinfo* RTTI_base_ ## cls [] = 		\
	       { UL_STATIC_TYPE_INFO(b1).get_info(),			\
		 UL_STATIC_TYPE_INFO(b2).get_info(),			\
		 UL_STATIC_TYPE_INFO(b3).get_info(), NULL };		\
  	void* cls::RTTI_cast(ulRTTItypeid t)				\
	{								\
	   if (t.isSame(&RTTI_obj)) return this;			\
	   void* ptr;							\
	   if ((ptr=b1::RTTI_cast(t))) return ptr;			\
	   if ((ptr=b2::RTTI_cast(t))) return ptr;			\
	   if ((ptr=b3::RTTI_cast(t))) return ptr;			\
	   return NULL;							\
	}								\
	void* cls::RTTI_scast(int i,void* p)				\
	{  cls* ptr = (cls*)p;						\
	   switch(i)							\
	   {  case  0: return (b1*)ptr;					\
	      case  1: return (b2*)ptr;					\
	      case  2: return (b3*)ptr;					\
	   }								\
	   return ptr;							\
	}							



#define UL_RTTI_DEF_INST(cls)		\
	UL_RTTI_DEF_BASE(cls)		\
	UL_RTTI_NEW(cls)

#define UL_RTTI_DEF(cls)		\
	UL_RTTI_DEF_BASE(cls)		\
	UL_RTTI_NO_NEW(cls)

#define UL_RTTI_DEF1_INST(cls,b1)	\
	UL_RTTI_DEF1_BASE(cls,b1)	\
	UL_RTTI_NEW(cls)

#define UL_RTTI_DEF1(cls,b1)		\
	UL_RTTI_DEF1_BASE(cls,b1)	\
	UL_RTTI_NO_NEW(cls)
	
#define UL_RTTI_DEF2_INST(cls,b1,b2)	\
	UL_RTTI_DEF2_BASE(cls,b1,b2)	\
	UL_RTTI_NEW(cls)

#define UL_RTTI_DEF2(cls,b1,b2)		\
	UL_RTTI_DEF2_BASE(cls,b1,b2)	\
	UL_RTTI_NO_NEW(cls)

#define UL_RTTI_DEF3_INST(cls,b1,b2,b3)	\
	UL_RTTI_DEF3_BASE(cls,b1,b2,b3)	\
	UL_RTTI_NEW(cls)

#define UL_RTTI_DEF3(cls,b1,b2,b3)	\
	UL_RTTI_DEF3_BASE(cls,b1,b2,b3)	\
	UL_RTTI_NO_NEW(cls)


#endif

