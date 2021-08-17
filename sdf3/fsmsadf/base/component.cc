/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   component.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 22, 1009
 *
 *  Function        :   Base component object
 *
 *  History         :
 *      22-04-09    :   Initial version.
 *
 * $Id: component.cc,v 1.1.2.2 2010-04-22 07:07:10 mgeilen Exp $
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

#include "component.h"

namespace FSMSADF
{

    /**
     * GraphComponent ()
     * Constructor.
     */
    GraphComponent::GraphComponent(GraphComponent *parent, const CId id)
        :
        parent(parent),
        id(id)
    {
    }

    /**
     * GraphComponent ()
     * Constructor.
     */
    GraphComponent::GraphComponent(GraphComponent *parent, const CId id,
                                   const CString &name)
        :
        parent(parent),
        id(id),
        name(name)
    {
    }

} // End namespace FSMSADF

