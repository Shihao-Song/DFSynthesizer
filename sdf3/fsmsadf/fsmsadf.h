/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   fsmsadf.h
 *
 *  Author          :   Sander Stuijk (s.stuijk@tue.nl)
 *
 *  Date            :   21 April 2009
 *
 *  Function        :   FSM-based SADF Graph
 *
 *  History         :
 *      21-04-09    :   Initial version.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */

#ifndef FSMSADF_FSMSADF_H_INCLUDED
#define FSMSADF_FSMSADF_H_INCLUDED

// Basic graph definitions of the FSMSADF MoC
#include "base/base.h"
#include "base/actor.h"
#include "base/channel.h"
#include "base/graph.h"
#include "base/port.h"

// Analysis algorithms
#include "analysis/analysis.h"

// Random graph generation
#include "generate/generate.h"

// Resource allocation (design flow)
#include "resource_allocation/flow/base_platform/base/flow.h"
#include "resource_allocation/flow/virtual_platform/base/flow.h"
#include "resource_allocation/flow/compsoc_platform/base/flow.h"
#include "resource_allocation/flow/mamps_platform/base/flow.h"

/**
 * Defines the type of platform.
 */
enum PlatformType
{
    PlatformTypeVirtual,
    PlatformTypeCompSoC,
    PlatformTypeMAMPS,
    PlatformTypeUnknown
};

// Transformation algorithms
#include "transform/transform.h"

// Output FSM-based SADF in HTML format
#include "output/html/html.h"

#endif
