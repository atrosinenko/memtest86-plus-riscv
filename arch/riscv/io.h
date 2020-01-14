#ifndef IO_H
#define IO_H

#define IN(tpe, addr, off) *(volatile tpe*)((uint8_t*)(addr) + off)
#define OUT(tpe, addr, off, data) *(volatile tpe*)((uint8_t*)(addr) + off) = (data)

#endif
