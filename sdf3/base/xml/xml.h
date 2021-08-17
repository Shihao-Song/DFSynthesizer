/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   xml.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 29, 2002
 *
 *  Function        :   XML interface functions
 *
 *  History         :
 *      29-03-02    :   Initial version.
 *      26-08-06    :   Additions by Bart Theelen.
 *            08    :   ported to MS compatibility by Marc Geilen
 *      11-04-11    :   Add libxml/xmlschemas.h for validation support.
 *
 * $Id: xml.h,v 1.3.2.6 2010-08-15 19:30:04 mgeilen Exp $
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

#ifndef BASE_XML_XML_H_INCLUDED
#define BASE_XML_XML_H_INCLUDED

#include "../string/cstring.h"

#ifdef _MSC_VER

#include "tchar.h"

#import <msxml6.dll> named_guids
using namespace MSXML2;

typedef MSXML2::IXMLDOMNodePtr                  XmlNode;
typedef MSXML2::IXMLDOMDocument2Ptr             XmlDocument;
typedef MSXML2::IXMLDOMElementPtr               XmlElement;
typedef MSXML2::IXMLDOMAttributePtr             XmlAttribute;
typedef MSXML2::IXMLDOMCommentPtr               XmlComment;
typedef MSXML2::IXMLDOMNamedNodeMapPtr          XmlNamedNodeMap;
typedef MSXML2::IXMLDOMNodeListPtr              XmlNodeList;
typedef MSXML2::IXMLDOMDocumentFragmentPtr      XmlDocumentFragment;
typedef MSXML2::IXMLDOMCDATASectionPtr          XmlCDataSection;
typedef MSXML2::IXMLDOMProcessingInstructionPtr XmlProcessingInstruction;
typedef MSXML2::IXMLDOMSchemaCollectionPtr      XmlSchemaCollection;
typedef MSXML2::IXMLDOMSchemaCollectionPtr      XmlSchemaCollection;
typedef MSXML2::IXMLDOMParseErrorPtr            XmlParseError;
typedef MSXML2::IXSLProcessorPtr                XslProcessor;
typedef MSXML2::IXSLTemplatePtr                 XslTemplate;

#define CLSID_XmlDocument                       __uuidof(MSXML2::DOMDocument60)
#define CLSID_XmlSchemaCache                    __uuidof(MSXML2::XMLSchemaCache60)
#define CLSID_XmlDomDocument                    __uuidof(MSXML2::DOMDocument60)

class CNode
{
    private:
        friend class CDoc;
        XmlNode  realNode;
        CNode(XmlNode n);
        XmlNode findNode(XmlNode startNode, const CString &name);
        static XmlDocument nodeFactory();
        static XmlDocument xmlNodeFactory;
    public:
        CNode(const CString &name);
        CString getName() const;
        CNode *parent() const;
        bool hasChildNode(const CString &name);
        CNode *findChildNode(const CString &name);
        CNode *findNode(const CString &name);
        CString getContent();
        void setContent(const CString &text);
        CString getXML();
        CNode *addChildNode(const CString &name, const CString &content);
        CNode *addChildNode(CNode *child);
        CNode *copy();
        CNode *replace(CNode *newNode);
        CNode *nextNode(const CString &name) const;
        bool hasAttribute(const CString &name);
        CString getAttribute(const CString &name);
        void addAttribute(const CString &name, const CString &value);
        void removeAttribute(const CString &name);
};
typedef CNode  *CNodePtr;

class CDoc
{
    private:
        friend class CNode;
        XmlDocument realDoc;
        CDoc(XmlDocument d);
    public:
        CDoc();
        CDoc(const CString &filename);
        void setRootNode(CNode *n);
        CNode *getRootNode();
        void save(const CString &filename);
        void addProcessingInstruction(const CString &pi, const CString &piArgs);
        bool validateSchema();
};
typedef CDoc *CDocPtr;

#else

#include <libxml/parser.h>
#include <libxml/xmlschemas.h>

// Types
typedef xmlNode     CNode;
typedef xmlNodePtr  CNodePtr;

typedef xmlAttr     CAttr;
typedef xmlAttrPtr  CAttrPtr;

typedef xmlDoc      CDoc;
typedef xmlDocPtr   CDocPtr;


#endif

/*
 * Functions
 */
// Open a file and parse it into a tree structure
CDoc *CParseFile(const CString &filename);

void CSetPrivateDTD(CDoc *doc, const CString &root_element, const CString &uri);

// Create a new document
CDoc *CNewDoc();

// Create a new document with root node
CDoc *CNewDoc(CNode *rootNode);

// Set doc root node
void CSetRootNode(CDoc *doc, CNode *rootNode);


// Output a document to a file
void CSaveFile(const CString &filename, CDoc *doc, int format = 0);

// Output a document to a stream
void CSaveFile(ostream &out, CDoc *doc, int format = 0);

// Get the root node of the tree
CNode *CGetRootNode(CDoc *doc);

// Return name of node
CString CIsNode(const CNode *n);

// Check that node is this node
bool CIsNode(const CNode *n, const CString &name);

// Return pointer to parent node
CNode *CGetParentNode(CNode *n);

// Get node with name name
CNode *CGetChildNode(CNode *n, const CString &name = "");

// Check that node has a child with name name
bool CHasChildNode(CNode *n, const CString &name = "");

// Return content of node as string
CString CGetNodeContent(CNode *n);

// Add a sub-tree
CNode *CAddNode(CNode *n, const CString &name,
                const CString &content = "");
CNode *CAddNode(CNode *n, const CString &name,
                const double content);
CNode *CAddNode(CNode *n, CNode *child);

// Create a copy of a node (including attributes and children)
CNode *CCopyNode(CNode *n);

// Create a new node
CNode *CNewNode(const CString &name);

// Remove node and all child nodes
void CRemoveNode(CNode *n);

// Replace node with new node (old node is returned)
CNode *CReplaceNode(CNode *oldNode, CNode *newNode);

// Find node with name (search in childs of node)
CNode *CFindNode(CNode *n, const CString &name);

// Get next node on same level (if name is not NULL,
// next node must have name 'name')
CNode *CNextNode(const CNode *n, const CString &name = "");

// Get previous node on same level (if name is not NULL,
// previous node must have name 'name')
CNode *CPreviousNode(const CNode *n, const CString &name = "");

// Get value of attribute
CString CGetAttribute(CNode *n, const CString &name);

// Check that attribute is present
bool CHasAttribute(CNode *n, const CString &name);

// Add an attribute to the node
void CAddAttribute(CNode *n, const CString &name, const CString &value);

// Set or reset an attribute
void CSetAttribute(CNode *n, const CString &name, const CString &value);

// Remove attribute from node
void CRemoveAttribute(CNode *n, const CString &name);

// Set content of node
void CSetContent(CNode *n, const CString &cont);

// Get number of child nodes with name name
CId CGetNumberOfChildNodes(CNode *n, const CString &name);

// free the xml doc
void releaseDoc(CDoc *doc);

// call at the end to release xml related resources.
void xmlCleanup();

// Adds a processing instuction to the document such as a stylesheet
void CAddProcessingInstruction(CDoc *doc, const CString &pi, const CString &piArgs);

// Validate the current document.
bool CValidate(CDoc *doc);
#endif

