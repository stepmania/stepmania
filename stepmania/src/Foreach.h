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

#endif
