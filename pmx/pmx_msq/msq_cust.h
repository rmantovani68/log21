/* msq_cust.h
 * header file for custom definitions
 * of message queues
 * Created by John Thornton, 27.01.93
 */
#ifndef _MSQ_CUST_H
#define _MSQ_CUST_H

#define KEY_MAIN_MODULE		0x10
/* This can't be zero, otherwise we shall be sending a
 * message type of zero, which leads to error EINVAL
 */
#define GID_MAIN_MODULE		1

#endif	/* _MSQ_CUST_H */
