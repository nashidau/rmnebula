#ifndef DEBUG_H_
#define DEBUG_H_

#ifndef NDEBUG
#define debug(x) printf x
#else
#define debug(x) 0
#endif

#endif /* DEBUG_H_ */
