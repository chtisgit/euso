/*!
	\file	childlist.h
	\author	Christian Fiedler <e1363562@student.tuwien.ac.at>
	\date	16.11.2015

	\brief	header file for childlist.c
	
	\details
		this module is a library that provides functions
		to save information about child processes
*/

#ifndef __PIDLIST_H__
#define __PIDLIST_H__
#include <unistd.h>

/*! \brief	Info structure for children of main process */
struct ChildInfo{

	/*! pid of child */
	pid_t pid;

	/*! password that shall be hashed */
	char *pw;

	/*! active=0 means the child has exited */
	int active;

	/*! exit status is saved when the child exits */
	int status;
};

/*! \brief	dynamic array of ChildInfo structures */
struct ChildList{
	/*! buffer to elements of the list */
	struct ChildInfo *buf;

	/*! number of preallocated slots for struct ChildInfo's*/
	size_t maxlen;
	
	/*! count of elements currently saved in the list */
	size_t len;
};

/*!
	\brief	creates a new ChildList
	\return	pointer to dynamically allocated struct ChildList, that
		is initialized or NULL if the function failed
	\param	init_len
		initial pre-alloced size of the list
*/
struct ChildList* childlist_new(size_t init_len);

/*!
	\brief	adds a ChildInfo struct to a ChildList
	\return	0 on error and 1 otherwise
	\param	pl
		pointer to ChildList structure that info shall be appended to
	\param	info
		pointer to a ChildInfo structure that shall be copied
		into the ChildList pl
*/
int childlist_add(struct ChildList *pl, const struct ChildInfo *const info);

/*!
	\brief	removes a ChildInfo struct from a ChildList by position
	\param	pl
		pointer to ChildList structure that info shall be appended to
	\param	info
		pointer to a ChildInfo structure that shall be copied
		into the ChildList pl
	
	the position must be greater than or equal to 0 and less than
	childlist_len(pl). Bounds checking is only done in debug mode.
*/
void childlist_remove(struct ChildList *pl, size_t pos);

/*!
	\brief	frees all memory of a ChildList structure
	\param	pl
		pointer to list that shall be freed
	
	the pointer must point to a valid ChildList object
*/
void childlist_delete(struct ChildList *pl);

/*!
	\brief	returns a copy of an element in a childlist based on position
	\param	pl
		pointer to list that shall be used
	\param	pos
		position of desired element in list pl
*/
struct ChildInfo childlist_get(struct ChildList *pl, size_t pos);

/*!
	\brief	returns a const pointer to an element in a childlist based on position
	\param	pl
		pointer to list that shall be used
	\param	pos
		position of desired element in list pl
*/
const struct ChildInfo* childlist_get_ro(struct ChildList *pl, size_t pos);

/*!
	\brief	marks ChildInfo elements with pid as inactive
	\param	pl
		pointer to list that shall be used
	\param	pos
		position of desired element in list pl
	\param	status
		exit status to be saved

	this function searches the list pl for elements with pid pid,
	marks them as inactive and saves their exit status
*/
void childlist_mark_inactive(struct ChildList *pl, pid_t pid, int status);

/*!
	\brief	returns the number of elements in a ChildList
	\param	pl
		pointer to list that shall be used
*/
static inline int childlist_len(struct ChildList *pl)
{
	return pl->len;
}


#endif

