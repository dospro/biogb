/*
 *     Proyect: BioGB
 *    Filename: cLibAO.h
 *     Version: v4.0
 * Description: Gameboy Color Emulator
 *     License: GPLv2
 *
 *      Author: Copyright (C) Rub� Daniel Guti�rez Cruz <dospro@gmail.com>
 *        Date: 07-1-2007
 *
 *
 *	This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details. 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */


#ifndef BIOGB_LIBAO
#define BIOGB_LIBAO

#include<ao/ao.h>
#include<string.h>
#include"../../cSound.h"


class cLibAO : public cSound {
private:
    ao_device *device;

public:
    cLibAO();
    virtual ~cLibAO();

    virtual bool init(u32 freq, u32 size, u32 bSize);
    virtual void turnOn(void);
    virtual void turnOff(void);
    virtual void play(void);
};

#endif
