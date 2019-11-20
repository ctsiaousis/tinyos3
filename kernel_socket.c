
#include "tinyos.h"


Fid_t sys_Socket(port_t port)
{
	/*fcb_reserv me ena (anti gia 2, pipe)
	sto fcb bazo to SCB kai to socket_func*/
	return NOFILE;
}

int sys_Listen(Fid_t sock)
{
	return -1;
}


Fid_t sys_Accept(Fid_t lsock)
{
	return NOFILE;
}


int sys_Connect(Fid_t sock, port_t port, timeout_t timeout)
{
	return -1;
}


int sys_ShutDown(Fid_t sock, shutdown_mode how)
{
	return -1;
}

/*
o listener kalei tin socket ki auti kalei tin
bind() pou deixnei to port tou listener basei
tou port table pou ulopoiisame.
epeita kaleite i
listen() kai brisketai se katastasi anamonis perimenontas
nees sindeseis meso tis accept().

o listener krataei ola ta requests kai ta eksupiretei seiriaka
otan sundethei kapoios akolouthei mia diadikasia write kai read.
ara dimiourgo tin socket_write() kai socket_read() pou aksiopoiei 
ton mixanismo ton pipes.
otan teleiosei h diadikasia o requester kalei tin close kai 
auti stelnei EOF ston listener etsi oste na kalesei ki autos close()
an kai mono an exei eksupiretisei ola ta requests!!!

ta sockets einai listener, an akouei
				 peer, an exei connection (mporei kai listener tautoxrona)
				 unbound, an den exei sundethei

Prepei na uparxei elegxos gia to type tou socket
Prepei na balo union sto SCB gia diaforetiki ulopoiisi
analoga me listener kai peer. to type dld. 
*/