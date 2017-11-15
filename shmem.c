// Coded by Chikashi Miyama

#include "ext.h"							// standard Max include, always required
#include "ext_obex.h"						// required for new style Max object
#include "buffer.h"

#include <stdio.h> 
#include <windows.h>						// on unix sys/shm.h
#include <sys/stat.h> 
#include <tchar.h>
#include <time.h>

typedef struct _shmem{
  t_object x_obj;
  long length;
  long datasize;
  HANDLE hMapFile;
} t_shmem;

// prototype declarations
void *shmem_new(t_symbol* shmemName, long size);
void shmem_free(t_shmem *x);
void shmem_write(t_shmem *x, t_symbol* bufName);
void shmem_read(t_shmem *x, t_symbol* bufName);
t_buffer_obj * shmem_getObj(t_shmem *x, t_symbol *bufName);

// pointer to the class
void *shmem_class;

void ext_main(void *r) {
	t_class *c;

	c = class_new("shmem", (method)shmem_new, (method)shmem_free, (long)sizeof(t_shmem),
		0L /* leave NULL!! */, A_SYM, A_LONG, 0);

	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	class_addmethod(c, (method)shmem_read, "read", A_SYM, 0);
	class_addmethod(c, (method)shmem_write, "write", A_SYM, 0);
	shmem_class = c;

}

void *shmem_new(t_symbol *shmemName, long length){
	if (length <= 0) {
		post("shmem: size too small");
		return NULL;
	}

	t_shmem *x;									// local variable (pointer to a t_plussz data structure)
	x = (t_shmem *)object_alloc(shmem_class);	// create a new instance of this object
	x->length = length;
	x->datasize = length * sizeof(float);
	TCHAR * objectName = TEXT(shmemName->s_name);
	x->hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,   // use paging file
		NULL,                   // default security
		PAGE_READWRITE,         // read/write access
		0,                      // maximum object size (high-order DWORD)
		x->length,				// maximum object size (low-order DWORD)
		objectName);			// name of mapping object

	if (x->hMapFile == NULL) {
		post("Could not create file mapping object (error:%d).\n", GetLastError());
		return NULL;
	}
	return(x);
}

void shmem_free(t_shmem *x) {
	CloseHandle(x->hMapFile);
}


// copy buffer to shared mem
void shmem_write(t_shmem *x, t_symbol *bufName) {
	t_buffer_obj *bufObj = shmem_getObj(x, bufName);
	float* pBuf = (float*)MapViewOfFile(x->hMapFile, FILE_MAP_WRITE, 0, 0, x->length);

	///////////// critical session
	float* bufPtr = buffer_locksamples(bufObj);
	CopyMemory(pBuf, bufPtr, x->datasize);
	buffer_unlocksamples(bufObj);
	///////////// critical session

	UnmapViewOfFile(pBuf);
}

// copy buffer from shared mem
void shmem_read(t_shmem *x, t_symbol *bufName) {
	t_buffer_obj *bufObj =  shmem_getObj(x, bufName);
	float* pBuf = (float*)MapViewOfFile(x->hMapFile, FILE_MAP_READ, 0, 0, x->length);

	///////////// critical session
	float* bufPtr = buffer_locksamples(bufObj);
	CopyMemory(bufPtr, pBuf, x->datasize);
	buffer_unlocksamples(bufObj);
	///////////// critical session

	UnmapViewOfFile(pBuf);
	buffer_setdirty(bufObj);
}

t_buffer_obj * shmem_getObj(t_shmem *x, t_symbol *bufName) {
	t_buffer_ref* bufRef = buffer_ref_new((t_object*)x, bufName);
	if (!buffer_ref_exists(bufRef)) {
		post("buffer~ %s not found", bufName->s_name);
		object_free(bufRef);
		return NULL;
	}

	t_buffer_obj *bufObj = buffer_ref_getobject(bufRef);
	if (buffer_getchannelcount(bufObj) != 1) {
		post("buffer~ %s must be a single channel buffer", bufName->s_name);
		object_free(bufRef);
		return NULL;
	}

	t_atom_long frameCount = buffer_getframecount(bufObj);
	if (frameCount != x->length) {
		post("shmem size doesn't match", bufName->s_name);
		object_free(bufRef);
		return NULL;
	}

	object_free(bufRef);
	return bufObj;
}
