/*
=================================================================
Copyright (C) 2005-2006 Torben "ntba2" Koenke

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
MA  02110-1301, USA.
=================================================================
*/

// Taken from 'EE timers' article written by shawn_t

#include <stdio.h>
#include <tamtypes.h>
#include <kernel.h>

#ifndef _TIMER_H
#define _TIMER_H

//
// Defines and Enumerations for timer related tasks
//

#define T0_COUNT		((volatile unsigned long*)0x10000000)
#define T0_MODE			((volatile unsigned long*)0x10000010)
#define T0_COMP			((volatile unsigned long*)0x10000020)
#define T0_HOLD			((volatile unsigned long*)0x10000030)

#define T1_COUNT		((volatile unsigned long*)0x10000800)
#define T1_MODE			((volatile unsigned long*)0x10000810)
#define T1_COMP			((volatile unsigned long*)0x10000820)
#define T1_HOLD			((volatile unsigned long*)0x10000830)

// Note! T2 and T3 don't have a Tn_HOLD register!
//
#define T2_COUNT		((volatile unsigned long*)0x10001000)
#define T2_MODE			((volatile unsigned long*)0x10001010)
#define T2_COMP			((volatile unsigned long*)0x10001020)

#define T3_COUNT		((volatile unsigned long*)0x10001800)
#define T3_MODE			((volatile unsigned long*)0x10001810)
#define T3_COMP			((volatile unsigned long*)0x10001820)

#define Tn_MODE(CLKS,GATE,GATS,GATM,ZRET,CUE,CMPE,OVFE,EQUF,OVFF) \
	 (u32)((u32)(CLKS)  | ((u32)(GATE) << 2) | \
	((u32)(GATS) << 3)  | ((u32)(GATM) << 4) | \
	((u32)(ZRET) << 6)  | ((u32)(CUE) << 7)  | \
	((u32)(CMPE) << 8)  | ((u32)(OVFE) << 9) | \
	((u32)(EQUF) << 10) | ((u32)(OVFF) << 11))

#define kBUSCLK				(147456000)
#define kBUSCLKBY16			(kBUSCLK / 16)
#define kBUSCLKBY256		(kBUSCLK / 256)
#define kHBLNK_NTSC			(15734)
#define kHBLNK_PAL			(15625)
#define kHBLNK_DTV480p		(31469)
#define kHBLNK_DTV1080i		(33750)

enum
{
	kINTC_GS,
	kINTC_SBUS,
	kINTC_VBLANK_START,
	kINTC_VBLANK_END,
	kINTC_VIF0,
	kINTC_VIF1,
	kINTC_VU0,
	kINTC_VU1,
	kINTC_IPU,
	kINTC_TIMER0,
	kINTC_TIMER1
};

#endif

//
// Static Timer Variable
//

static int      s_tnInterruptID		= -1;
static u64      s_tnInterruptCount	= 0;

//
// Time Interrupt Callback
//

int tnTimeInterrupt( int ca )
{
	s_tnInterruptCount++;

	// A write to the overflow flag will clear the overflow flag
	//
	*T1_MODE |= (1 << 11);

	return -1;
}

//
// Time functions
//

void tnTimeInit( void )
{
	// ============================================================
	// I am using 1/256 of the BUSCLK below in the Tn_MODE register
	// which means that the timer will count at a rate of:
	//   147,456,000 / 256 = 576,000 Hz
	// This implies that the accuracy of this timer is:
	//   1 / 576,000 = 0.0000017361 seconds (~1.74 usec!)
	// The Tn_COUNT registers are 16 bit and overflow in:
	//   1 << 16 = 65536 seconds
	// This implies that our timer will overflow in:
	//   65536 / 576,000 = 0.1138 seconds
	// I use an interrupt to recognize this overflow and increment
	// the <s_tnInterruptCount> variable so I can easily compute
	// the total time. This results in a very accurate timer that
	// is also very efficient. It is possible to have an even more
	// accurate timer by modifying the Tn_MODE, but at the expense
	// of having to call the overflow interrupt more frequently.
	// For example, if you wanted to use 1/16 of the BUSCLK, the
	// timer would count at a rate of:
	//   147,456,000 / 16 = 9,216,000 Hz
	// which implies an accuracy of:
	//   1 / 9,216,000 = 0.0000001085 seconds (0.11 usec!)
	// However, the timer will overflow in:
	//   65536 / 9,216,000 = 0.0071 seconds (7.1 msec)
	// meaning, the interrupt would be called more then 140 times a
	// second. For my purposes the accuracy of ~1.74 usec is fine!
	// ============================================================

	// Disable T1_MODE
	*T1_MODE = 0x0000;

	// Initialize the overflow interrupt handler.
	s_tnInterruptID = AddIntcHandler(kINTC_TIMER1, tnTimeInterrupt, 0);
	EnableIntc(kINTC_TIMER1);

	// Initialize the timer registers
	// CLKS: 0x02 - 1/256 of the BUSCLK (0x01 is 1/16th)
	//  CUE: 0x01 - Start/Restart the counting
	// OVFE: 0x01 - An interrupt is generated when an overflow occurs
	*T1_COUNT	= 0;
	*T1_MODE	= Tn_MODE(0x02, 0, 0, 0, 0, 0x01, 0, 0x01, 0, 0);

	s_tnInterruptCount = 0;
}

//
// tnTimeUsec - Returns uptime in usec precision
//

u64 tnTimeUsec( void )
{
	u64	t;

	// Tn_COUNT is 16 bit precision. Therefore, each
	// <s_tnInterruptCount> is 65536 ticks
	t = *T1_COUNT + (s_tnInterruptCount << 16);

	t = t * 1000000 / kBUSCLKBY256;

	return t;
}

//
// tnTimeMsec - Returns uptime in msec precision
//

u64 tnTimeMsec( void )
{
	return tnTimeUsec() / 1000;
}

void tnTimeFini( void )
{
	// Stop the timer
	*T1_MODE = 0x0000;

	// Disable the interrupt
	// ---------------------
	if( s_tnInterruptID >= 0 )
	{
		DisableIntc(kINTC_TIMER1);
		RemoveIntcHandler(kINTC_TIMER1, s_tnInterruptID);
		s_tnInterruptID = -1;
	}
	
	s_tnInterruptCount = 0;
}
