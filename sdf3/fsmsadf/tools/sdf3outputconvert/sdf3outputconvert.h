/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3convert.h
 *
 *  Author          :   Martijn Koedam (m.l.p.j.koedam@tue.nl)
 *
 *  Date            :   6 April 2011
 *
 *  Function        :   Input file conversion
 *
 *  History         :
 *      19-04-11    :   Initial version.
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

#ifndef FSMSADF_SDF3CONVERT_H_INCLUDED
#define FSMSADF_SDF3CONVERT_H_INCLUDED

/**
 * Tool name and version
 */
#define TOOL               "sdf3outputconvert-fsmsadf"
#define DOTTED_VERSION     "0.01"
#define VERSION             100

/**
 * Module (MoC) supported by the tool
 */
#define MODULE             "fsmsadf"

/**
 * Settings type used by the tool
 */
#define SETTINGS_TYPE      "output-convert"

#include <iostream>
#include "../base/tools.h"
#include "../../resource_allocation/flow/mamps_platform/conversion/input_conversion.h"
#include "../../resource_allocation/flow/compsoc_platform/conversion/input_conversion.h"
#include "../../resource_allocation/flow/virtual_platform/conversion/input_conversion.h"
#include "../../resource_allocation/flow/base_platform/conversion/output_conversion.h"
#include "../../resource_allocation/flow/virtual_platform/conversion/output_conversion.h"
#include "../../resource_allocation/flow/compsoc_platform/conversion/output_conversion.h"
#include "../../resource_allocation/flow/mamps_platform/conversion/output_conversion.h"



#endif
