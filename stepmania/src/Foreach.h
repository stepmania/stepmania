#ifndef Foreach_H
#define Foreach_H

#define FOREACH( elemType, vect, var ) 			\
for( vector<elemType>::iterator var = (vect).begin(); var != (vect).end(); ++var )
#define FOREACH_CONST( elemType, vect, var ) 	\
for( vector<elemType>::const_iterator var = (vect).begin(); var != (vect).end(); ++var )

#define FOREACHD( elemType, vect, var ) 			\
for( deque<elemType>::iterator var = (vect).begin(); var != (vect).end(); ++var )
#define FOREACHD_CONST( elemType, vect, var ) 	\
for( deque<elemType>::const_iterator var = (vect).begin(); var != (vect).end(); ++var )

#define FOREACHS( elemType, vect, var ) 			\
for( set<elemType>::iterator var = (vect).begin(); var != (vect).end(); ++var )
#define FOREACHS_CONST( elemType, vect, var ) 	\
for( set<elemType>::const_iterator var = (vect).begin(); var != (vect).end(); ++var )

#define FOREACHMM( keyType, valType, vect, var ) 			\
for( multimap<keyType, valType>::iterator var = (vect).begin(); var != (vect).end(); ++var )

#endif
