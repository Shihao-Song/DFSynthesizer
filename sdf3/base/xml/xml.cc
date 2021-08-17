/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   xml.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 29, 2002
 *
 *  Function        :   XML interface functions
 *
 *  History         :
 *      29-03-02    :   Initial version.
 *      27-05-04    :   Changed datatypes to CAST datatypes.
 *      26-08-06    :   Additions by Bart Theelen.
 *      12-04-11    :   Add validation to libxml version.
 *
 * $Id: xml.cc,v 1.4.2.11 2010-08-15 19:30:04 mgeilen Exp $
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

#include "xml.h"
#include "../exception/exception.h"
#include "../tempfile/tempfile.h"
#include "../log/log.h"

#ifdef _MSC_VER

#define CHECK_HR_THROW(hr) { if (FAILED(hr)) { throw -1; } }

XmlDocument CNode::xmlNodeFactory;

CDoc::CDoc()
{
    HRESULT m_hLastError;
    m_hLastError = CoInitialize(NULL);
    CHECK_HR_THROW(m_hLastError);
    m_hLastError = CoCreateInstance(CLSID_XmlDocument, NULL,
                                    CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument2,
                                    (void **)&realDoc);
    CHECK_HR_THROW(m_hLastError);
}

CDoc::CDoc(const CString &filename)
{
    //Step 1: Create MSXML DOMDocument Object
    HRESULT m_hLastError;
    m_hLastError = CoInitialize(NULL);
    CHECK_HR_THROW(m_hLastError);
    m_hLastError = CoCreateInstance(CLSID_XmlDocument, NULL,
                                    CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument2,
                                    (void **)&realDoc);
    CHECK_HR_THROW(m_hLastError);

    // Load file synchronously, avoid validation, avoid external references
    // resolution
    realDoc->async = VARIANT_FALSE;
    realDoc->validateOnParse = VARIANT_TRUE;
    realDoc->resolveExternals = VARIANT_FALSE;

    //Set the SelectionLanguage to XPath
    m_hLastError = realDoc->setProperty(_bstr_t(_T("SelectionLanguage")),
                                        _variant_t(_T("XPath")));
    CHECK_HR_THROW(m_hLastError);

    //Try to load the local XML file
    if (realDoc->load(_variant_t(filename.c_str())) != VARIANT_TRUE)
    {
        m_hLastError = E_FAIL;
        throw - 1;
    }
}

CDoc::CDoc(XmlDocument d)
{
    this->realDoc = d;
}

void CDoc::setRootNode(CNode *n)
{
    XmlNode newNode = n->realNode->cloneNode(VARIANT_TRUE);
    this->realDoc->appendChild(newNode);
}

void CDoc::addProcessingInstruction(const CString &pi, const CString &piArgs)
{
    // Create a procesing instruction.
    XmlProcessingInstruction newPI;
    newPI = this->realDoc->createProcessingInstruction(pi.c_str(), piArgs.c_str());
    this->realDoc->appendChild(newPI);
}


void CDoc::save(const CString &filename)
{
    this->realDoc->save(_variant_t(filename.c_str()));
}

CNode *CDoc::getRootNode()
{
    XmlNode de;
    de = this->realDoc->documentElement;
    return new CNode(de);
}

bool CDoc::validateSchema()
{

    IXMLDOMSchemaCollectionPtr pSchemaCache;
    HRESULT hr;
    hr = pSchemaCache.CreateInstance(CLSID_XmlSchemaCache);

    XmlDocument pSchemaDoc;
    hr = pSchemaDoc.CreateInstance(CLSID_XmlDomDocument);
    pSchemaDoc->async =  VARIANT_FALSE;

    XmlNode de = this->realDoc->documentElement;
    CString slurl;

    XmlNode nsn;
    nsn = de->attributes->getNamedItem("xsi:schemaLocation");
    if (nsn != NULL)
    {
        _bstr_t sl = nsn->Gettext();
        CStrings ns_url = CString(sl).split(' ');
        slurl = ns_url.back();
    }
    else
    {
        nsn = de->attributes->getNamedItem("xsi:noNamespaceSchemaLocation");
        if (nsn == NULL)
        {
            // no schema specified, do not validate
            return true;
        }
        slurl = CString(nsn->Gettext());
    }
    if (pSchemaDoc->load(slurl.c_str()) != VARIANT_TRUE)
    {
        logWarning("cannot load schema file.");
        return false;
    }

    //add schema to schema cache
    try
    {
        hr = pSchemaCache->add("uri:sdf3", pSchemaDoc.GetInterfacePtr());
        if (FAILED(hr))
        {
            logWarning("cannot add schema file.");
            return false;
        }
    }
    catch (_com_error &e)
    {
        logWarning("Schema validation failed, because schema is invalid according to MSXML: ");
        logWarning(e.Description());
        return false;
    }


    this->realDoc->schemas = pSchemaCache.GetInterfacePtr();

    //call validate
    XmlParseError pError;
    pError = this->realDoc->validate();

    if (pError->errorCode != S_OK)
    {
        _bstr_t parseError = _bstr_t("Document does not pass schema validation. Reason: ") + pError->Getreason();
        logWarning((char *)parseError);
        return false;
    }
    else
        return true;
}


void releaseDoc(CDoc *doc)
{
    delete doc;
}


CNode::CNode(XmlNode n)
{
    this->realNode = n;
}

CString CNode::getName() const
{
    return CString(this->realNode->nodeName);
}

CNode *CNode::parent() const
{
    // This may not be safe, depending on how the XML interface is
    // used. A new CNode is created!!!
    XmlNode nd;
    nd = this->realNode->parentNode;
    return new CNode(nd);
}

CNode *CNode::findChildNode(const CString &name)
{
    // This may not be safe, depending on how the XML interface is used. A new
    // CNode is created!!!

    // Check all children of node
    XmlNode childNode = this->realNode->firstChild;

    while (childNode != NULL)
    {
        // Compare name of node and searched name
        if (name.empty() || !name.compare(childNode->nodeName))
        {
            // Found a child with correct name
            return new CNode(childNode);
        }
        childNode = childNode->nextSibling;
    }

    // Did not find a child with correct name
    return NULL;
}

CNode *CNode::findNode(const CString &name)
{
    // This may not be safe, depending on how the XML interface is used. A new
    // CNode is created!!!
    return new CNode(this->findNode(this->realNode, name));
}

XmlNode CNode::findNode(XmlNode startNode, const CString &name)
{
    // Try to get node at this level
    if (! name.compare(startNode->nodeName))
    {
        return startNode;
    }
    XmlNode node = NULL;
    // Recursion - search the children
    XmlNode childNode = startNode->firstChild;
    while (childNode != NULL)
    {
        node = this->findNode(childNode, name);
        childNode = childNode->nextSibling;
    }
    return node;
}

bool CNode::hasChildNode(const CString &name)
{
    // Check all children of node
    XmlNode childNode = this->realNode->firstChild;
    while (childNode != NULL)
    {
        // Compare name of node and searched name
        if (name.empty() || !name.compare(childNode->nodeName))
        {
            // Found a child with correct name
            return true;
        }
        childNode = childNode->nextSibling;
    }
    // Did not find a child with correct name
    return false;
}

CString CNode::getContent()
{
    return *new CString(this->realNode->text);
}


/*
* Set the node's content. Will be encoded to make xml compliant.
*/
void CNode::setContent(const CString &text)
{
    this->realNode->text = text.c_str();
}

CString CNode::getXML()
{
    return *new CString(this->realNode->xml);
}

CNode *CNode::addChildNode(const CString &name, const CString &content)
{
    XmlNode n = this->realNode->ownerDocument->createNode(MSXML2::NODE_ELEMENT,
                name.c_str(), "");
    XmlNode n_text = this->realNode->ownerDocument->createTextNode(content.c_str());
    CNode *cn = new CNode(n);
    this->realNode->appendChild(n);
    n->appendChild(n_text);
    return cn;
}

CNode *CNode::addChildNode(CNode *child)
{
    XmlNode newNode = child->realNode->cloneNode(VARIANT_TRUE);
    this->realNode->appendChild(newNode);
    return child;
}

CNode *CNode::copy()
{
    XmlNode newNode;
    newNode = this->realNode->cloneNode(VARIANT_TRUE);
    return new CNode(newNode);
}

CNode::CNode(const CString &name)
{
    this->realNode = CNode::nodeFactory()->createNode(MSXML2::NODE_ELEMENT,
                     name.c_str(), "");
}

CNode *CNode::replace(CNode *newNode)
{
    this->realNode->parentNode->replaceChild(newNode->realNode, this->realNode);
    return this;
}


CNode *CNode::nextNode(const CString &name) const
{
    XmlNode childNode = this->realNode->nextSibling;

    // search for node named name
    while (childNode != NULL)
    {
        if (name.empty() || !name.compare(childNode->nodeName))
        {
            return new CNode(childNode);
        }
        childNode = childNode->nextSibling;
    }
    return NULL;
}


bool CNode::hasAttribute(const CString &name)
{
    return this->realNode->attributes->getNamedItem(name.c_str()) != NULL;
}

CString CNode::getAttribute(const CString &name)
{
    XmlNode aNode = this->realNode->attributes->getNamedItem(name.c_str());
    return *new CString(aNode->text);
}

void CNode::addAttribute(const CString &name, const CString &value)
{
    XmlNode n = this->realNode;
    XmlElement e(n);
    if (e)
    {
        e->setAttribute(name.c_str(), value.c_str());
    }
}

void CNode::removeAttribute(const CString &name)
{
    XmlNode n = this->realNode;
    XmlElement e(n);
    if (e)
    {
        e->removeAttribute(name.c_str());
    }
}

XmlDocument CNode::nodeFactory()
{
    XmlDocument f = xmlNodeFactory;
    HRESULT hr;
    if (f == NULL)
    {
        hr = CoInitialize(NULL);
        hr = CoCreateInstance(CLSID_XmlDocument, NULL,
                              CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument2,
                              (void **)&xmlNodeFactory);
    }
    return xmlNodeFactory;
}

/**
 * CParseFile ()
 * Open a file and parse it into a tree structure.
 */
CDoc *CParseFile(const CString &filename)
{
    try
    {
        CDoc *doc = new CDoc(filename);
        if (!CValidate(doc))
        {
            logWarning("The document '" + filename + "' is not valid.");
        }
        return doc;
    }
    catch (int)
    {
        return NULL;
    }
}

/**
 * CNewDoc ()
 * Create a new document.
 */
CDoc *CNewDoc()
{
    CDoc *doc;
    doc = new CDoc();
    return doc;
}

/**
 * CNewDoc ()
 * Create a new document.
 */
CDoc *CNewDoc(CNode *rootNode)
{
    CDoc *doc;
    doc = new CDoc();
    doc->setRootNode(rootNode);
    return doc;
}

/**
 * CSetRootNode ()
 * Set the document root node
 */
void CSetRootNode(CDoc *doc, CNode *rootNode)
{
    doc->setRootNode(rootNode);
}

/**
 * CSaveFile ()
 * Output a document to a file.
 */
void CSaveFile(const CString &filename, CDoc *doc, int format)
{
    doc->save(filename);
}

/**
 * CSaveFile ()
 * Output a document to a stream.
 */
void CSaveFile(ostream &out, CDoc *doc, int format)
{
    char c;

    // Temporary file
    CString tmpfile = tempFileName(".", "smart");

    // Store XML document in temp file
    doc->save(tmpfile);

    // Read temp file to stream
    ifstream f(tmpfile);
    while (f.get(c) && !f.eof())
    {
        out << c;
    };
    f.close();

    // Remove temporary file
    remove(tmpfile);
}

/**
 * CGetRootNode ()
 * Get the root node of the document.
 */
CNode *CGetRootNode(CDoc *doc)
{
    return doc->getRootNode();
}

/**
 * CIsNode ()
 * Return name of node.
 */
CString CIsNode(const CNode *n)
{
    return n->getName();
}

/**
 * CIsNode ()
 * Check that node has name 'name'.
 */
bool CIsNode(const CNode *n, const CString &name)
{
    if (! name.compare(n->getName()))
        return true;
    return false;
}

/**
 * CGetParentNode ()
 * Return pointer to parent of node.
 */
CNode *CGetParentNode(const CNode *n)
{
    return n->parent();
}

/**
 * CGetChildNode ()
 * Return pointer to node with name 'name' (if exists).
 */
CNode *CGetChildNode(CNode *n, const CString &name)
{
    if (n == NULL)
        return NULL;
    return n->findChildNode(name);
}

/**
 * CHasChildNode ()
 * Check that the node has a child with name 'name'.
 */
bool CHasChildNode(CNode *n, const CString &name)
{
    if (n == NULL)
        return NULL;
    return n->hasChildNode(name);
}

/**
 * CGetNodeContent ()
 * Return the content of a node as a string
 */
CString CGetNodeContent(CNode *n)
{
    return n->getContent();
}

/**
 * CAddNode ()
 * Add a child node to given node.
 */
CNode *CAddNode(CNode *n, const CString &name,
                const CString &content)
{
    return n->addChildNode(name, content);
}

/**
 * CAddNode ()
 * Add a child node to given node.
 */
CNode *CAddNode(CNode *n, const CString &name,
                const double content)
{
    char buf[256];

    // Convert content to string
    sprintf(buf, "%.10e", content);

    // Add node
    return CAddNode(n, name, CString(buf));
}

/**
 * CAddNode ()
 * Add a child node to given node.
 */
CNode *CAddNode(CNode *n, CNode *child)
{
    return n->addChildNode(child);
}

/**
 * CCopyNode ()
 * Create a copy of a node (including attributes and children).
 */
CNode *CCopyNode(CNode *n)
{
    return n->copy();
}

/**
 * CNewNode ()
 * Create a new node.
 */
CNode *CNewNode(const CString &name)
{
    return new CNode(name);
}

/**
 * CRemoveNode ()
 * Remove a node and its children from the database.
 */
void CRemoveNode(CNode *n)
{
    //// Free the memory for the subtree starting in this node
    delete n;
}

/**
 * CReplaceNode ()
 * Replace node with new node (old node is returned).
 */
CNode *CReplaceNode(CNode *oldNode, CNode *newNode)
{
    return oldNode->replace(newNode);
}

/**
 * CFindNode ()
 * Get a child node with name 'name'. The whole subtree is searched for
 * the node and not only the direct children as with the getNode function.
 */
CNode *CFindNode(CNode *n, const CString &name)
{
    return n->findNode(name);
}

/**
 * CNextNode ()
 * Return pointer to next node on same level. If name is not NULL, next
 * node must have name 'name'.
 */
CNode *CNextNode(const CNode *n, const CString &name)
{
    return n->nextNode(name);
}

/**
 * CGetAttribute ()
 * Get the value of an attribute.
 */
CString CGetAttribute(CNode *n, const CString &name)
{
    if (CHasAttribute(n, name))
    {
        return n->getAttribute(name);
    }
    else
    {
        std::cerr << CIsNode(n) << " " << name << std::endl;
        throw CException("(CGetAttribute) Node does not have requested attribute.");
    }
    return CString("");
}

/**
 * CHasAttribute ()
 * Check that attribute is present.
 */
bool CHasAttribute(CNode *n, const CString &name)
{
    return n->hasAttribute(name);
}

/**
 * CAddAttribute ()
 * Add an attribute to the node.
 */
void CAddAttribute(CNode *n, const CString &name, const CString &value)
{
    n->addAttribute(name, value);
}

/**
 * CAddAttribute ()
 * Add an attribute to the node.
 */
void CAddAttribute(CNode *n, const CString &name, const unsigned long value)
{
    char buf[256];

    // Convert value to string
    sprintf(buf, "%ld", value);

    // Add attribute
    CAddAttribute(n, name, CString(buf));
}

/**
 * CSetAttribute ()
 * Set or reset an attribute of the node.
 */
void CSetAttribute(CNode *n, const CString &name, const CString &value)
{
    n->addAttribute(name, value);
}

/**
 * CRemoveAttribute ()
 * Remove attribute from node.
 */
void CRemoveAttribute(CNode *n, const CString &name)
{
    n->removeAttribute(name);
}

/**
 * CSetContent ()
 * Set content of node.
 */
void CSetContent(CNode *n, const CString &cont)
{
    n->setContent(cont);
}


/**
 * CGetNumberOfChildNodes ()
 * Return the number of child nodes with name 'name'.
 */
CId CGetNumberOfChildNodes(CNode *n, const CString &name)
{
    CId NumberOfChildNodes = 0;

    if (n != NULL)
    {
        for (CNode *t = n->findChildNode(name); t != NULL; t = t->nextNode(name))
        {
            if (t != NULL)
            {
                NumberOfChildNodes++;
            }
        }
    }

    return NumberOfChildNodes;
}


/**
 * xmlCleanup()
 * call at the end to release xml related resources.
 */
void xmlCleanup()
{
}

/**
 * CAddProcessingInstruction
 * Adds a processing instuction to the document
 * such as a stylesheet
 */
void CAddProcessingInstruction(CDoc *doc, const CString &pi, const CString &piArgs)
{
    doc->addProcessingInstruction(pi, piArgs);
}

/**
 * CValidate()
 *
 * First validates using DTD the xml file, and or schema file.
 *
 * Make libxml validate the xml file using the specified schema (xi:schema)
 * Errors are printed out to sderr using the default libxml error callback.
 * CValidate() also reports final result to stderr.
 *
 * @param doc The CDoc of the document we want to validate.
 * @return true if document is valid, false if invalid
 */
bool CValidate(CDoc *doc)
{

    return doc->validateSchema();

}


#else

/**
 * Error handler
 */
#define TMP_BUF_SIZE 256
void err(void *ctx, const char *msg, ...)
{
    char cstring[TMP_BUF_SIZE];
    int len = 0;
    va_list arg_ptr;
    va_start(arg_ptr, msg);
    len = vsnprintf(cstring, TMP_BUF_SIZE, msg, arg_ptr);

    // strip of trailing newline.
    if (len > 0 && cstring[len - 1] == '\n')
        cstring[len - 1] = '\0';

    va_end(arg_ptr);
    logWarning(CString("XML: ") + (const char *)cstring);
}

/**
 * CValidate()
 *
 * First validates using DTD the xml file, and or schema file.
 *
 * Make libxml validate the xml file using the specified schema (xi:schema)
 * Errors are printed out to sderr using the default libxml error callback.
 * CValidate() also reports final result to stderr.
 *
 * @param doc The CDoc of the document we want to validate.
 * @return true if document is valid, false if invalid
 */
bool CValidate(CDoc *doc)
{

    if (doc == NULL)
    {
        logWarning("XML: No document specified (doc == NULL)");
        return false;
    }

    int res;
    if (xmlGetIntSubset(doc) != NULL)
    {
        /* Validate DTD */
        xmlValidCtxtPtr ctx = xmlNewValidCtxt();
        if (xmlValidateDocument(ctx, doc) == 0)
        {
            xmlFreeValidCtxt(ctx);
            return false;
        }
        xmlFreeValidCtxt(ctx);
    }

    CNode *root = CGetRootNode(doc);
    /* nothing to validate */
    if (root == NULL)
    {
        return true;
    }

    // hack to check if there is a schema file specified.
    // This hacks avoid that there is a warning.
    xmlChar *nsl = xmlGetProp(root, BAD_CAST"noNamespaceSchemaLocation");
    xmlChar *sl = xmlGetProp(root, BAD_CAST"schemaLocation");
    if (nsl == NULL && sl == NULL)
    {
        return true;
    }
    if (nsl) xmlFree(nsl);
    if (sl) xmlFree(sl);

    /* the XSD validation context */
    xmlSchemaValidCtxtPtr validationCtxt;


    /* A validation context created with NULL as the
     * @schema argument will validate the instance against the schemata
     * indicated by xsi:schemaLocation and xsi:noNamespaceSchemaLocation.
     */
    validationCtxt = xmlSchemaNewValidCtxt(NULL);
    if (validationCtxt == NULL)
    {
        logWarning("XML: Failed to create the validation context.");
        return false;
    }

    /* Validate the instance. */
    res = xmlSchemaValidateDoc(validationCtxt, doc);
    if (res == -1)
    {
        logWarning("XML: Internal error during validation.");
    }
    /* Cleanup */
    xmlSchemaFreeValidCtxt(validationCtxt);

    /* return result.*/
    return (res == 0);
}

/**
 * CParseFile ()
 * Open a file and parse it into a tree structure.
 */
CDoc *CParseFile(const CString &filename)
{
    xmlSetGenericErrorFunc(NULL, err);
    CDoc *doc = xmlReadFile(filename.c_str(), NULL, 0);

    if (doc != NULL)
    {

        if (!CValidate(doc))
        {
            logWarning("The document '" + filename + "' is not valid.");
        }
    }
    return doc;
}

/**
 * CSetPrivateDTD
 *
 * Set private (SYSTEM) dtd header on document.
 */
void CSetPrivateDTD(CDoc *doc, const CString &root_element, const CString &uri)
{
    xmlCreateIntSubset(doc,
                       BAD_CAST(const char *)root_element,
                       NULL,
                       BAD_CAST(const char *)uri);
}
/**
 * CNewDoc ()
 * Create a new document.
 */
CDoc *CNewDoc()
{
    CDoc *doc;

    doc = xmlNewDoc(BAD_CAST "1.0");
    return doc;
}


/**
 * CNewDoc ()
 * Create a new document.
 */
CDoc *CNewDoc(CNode *rootNode)
{
    CDoc *doc;

    doc = xmlNewDoc(BAD_CAST "1.0");
    xmlDocSetRootElement(doc, rootNode);

    return doc;
}

/**
 * CSaveFile ()
 * Output a document to a file.
 */
void CSaveFile(const CString &filename, CDoc *doc, int format)
{
    xmlSaveFormatFile(filename.c_str(), doc, format);
}

/**
 * CSaveFile ()
 * Output a document to a stream.
 */
void CSaveFile(ostream &out, CDoc *doc, int format)
{
    char c;

    // Temporary file
    CString tmpfile = tempFileName("", "sdf3");

    // Store XML document in temp file
    xmlSaveFormatFile(tmpfile, doc, format);

    // Read temp file to stream
    ifstream f(tmpfile);
    while (f.get(c) && !f.eof())
    {
        out << c;
    };
    f.close();

    // Remove temporary file
    remove(tmpfile);
}

/**
 * CGetRootNode ()
 * Get the root node of the document.
 */
CNode *CGetRootNode(CDoc *doc)
{
    return xmlDocGetRootElement(doc);
}

/**
 * CIsNode ()
 * Return name of node.
 */
CString CIsNode(const CNode *n)
{
    return CString((char *)n->name);
}

/**
 * CIsNode ()
 * Check that node has name 'name'.
 */
bool CIsNode(const CNode *n, const CString &name)
{
    if (!xmlStrcmp(n->name, (const xmlChar *) name.c_str()))
        return true;

    return false;
}

/**
 * CGetParentNode ()
 * Return pointer to parent of node.
 */
CNode *CGetParentNode(CNode *n)
{
    return n->parent;
}

/**
 * CGetChildNode ()
 * Return pointer to node with name 'name' (if exists).
 */
CNode *CGetChildNode(CNode *n, const CString &name)
{
    if (n == NULL)
        return NULL;

    // Check all children of node
    for (CNode *t = n->children; t != NULL; t = t->next)
    {
        // Compare name of node and searched name
        if (name.empty() || CIsNode(t, name))
        {
            // Found a child with correct name
            return t;
        }
    }

    // Did not find a child with correct name
    return NULL;
}

/**
 * CHasChildNode ()
 * Check that the node has a child with name 'name'.
 */
bool CHasChildNode(CNode *n, const CString &name)
{
    if (n == NULL)
        return false;

    // Check all children of node
    for (CNode *t = n->children; t != NULL; t = t->next)
    {
        // Compare name of node and searched name
        if (name.empty() || CIsNode(t, name))
        {
            // Found a child with correct name
            return true;
        }
    }

    // Did not find a child with correct name
    return false;
}

/**
 * CGetNodeContent ()
 * Return the content of a node as a string
 */
CString CGetNodeContent(CNode *n)
{
    CString contentStr;
    xmlChar *cont = xmlNodeGetContent(n);
    contentStr = (const char *) cont;
    xmlFree(cont);
    return contentStr;
}

/**
 * CAddNode ()
 * Add a child node to given node.
 */
CNode *CAddNode(CNode *n, const CString &name,
                const CString &content)
{
    CNode *r;

    if (n == NULL)
    {
        r = xmlNewNode(NULL, (const xmlChar *) name.c_str());
        xmlNodeSetContent(r, (const xmlChar *) content.c_str());
        return r;
    }

    // Add a child to the tree below node n
    r = xmlNewChild(n, NULL, (const xmlChar *) name.c_str(),
                    (const xmlChar *) content.c_str());

    return r;
}

/**
 * CAddNode ()
 * Add a child node to given node.
 */
CNode *CAddNode(CNode *n, const CString &name,
                const double content)
{
    char buf[256];

    // Convert content to string
    sprintf(buf, "%.10e", content);

    // Add node
    return CAddNode(n, name, CString(buf));
}

/**
 * CAddNode ()
 * Add a child node to given node.
 */
CNode *CAddNode(CNode *n, CNode *child)
{
    return xmlAddChild(n, child);
}

/**
 * CCopyNode ()
 * Create a copy of a node (including attributes and children).
 */
CNode *CCopyNode(CNode *n)
{
    return xmlCopyNode(n, 1);
}

/**
 * CNewNode ()
 * Create a new node.
 */
CNode *CNewNode(const CString &name)
{
    return xmlNewNode(NULL, (const xmlChar *)name.c_str());
}

/**
 * CRemoveNode ()
 * Remove a node and its children from the database.
 */
void CRemoveNode(CNode *n)
{
    // Unlink the node from the tree
    xmlUnlinkNode(n);

    // Free the memory for the subtree starting in this node
    xmlFreeNode(n);
}

/**
 * CReplaceNode ()
 * Replace node with new node (old node is returned).
 */
CNode *CReplaceNode(CNode *oldNode, CNode *newNode)
{
    return xmlReplaceNode(oldNode, newNode);
}

/**
 * CFindNode ()
 * Get a child node with name 'name'. The whole subtree is searched for
 * the node and not only the direct children as with the getNode function.
 */
CNode *CFindNode(CNode *n, const CString &name)
{
    CNode *node;

    if (n == NULL)
        return NULL;

    // Try to get node at this level
    node = CGetChildNode(n, name);

    // End of recursion
    if (node != NULL)
        return node;

    // Recursion - search the children
    for (CNode *t = n->children; t != NULL && node == NULL; t = t->next)
    {
        node = CFindNode(t, name);
    }

    return node;
}

/**
 * CNextNode ()
 * Return pointer to next node on same level. If name is not NULL, next
 * node must have name 'name'.
 */
CNode *CNextNode(const CNode *n, const CString &name)
{
    CNode *t;

    for (t = n->next; t != NULL; t = t->next)
    {
        if (name.empty() || CIsNode(t, name))
            return t;
    }

    return NULL;
}

/**
 * CPreviousNode ()
 * Return pointer to previous node on same level. If name is not NULL, previous
 * node must have name 'name'.
 */
CNode *CPreviousNode(const CNode *n, const CString &name)
{
    CNode *t;

    for (t = n->prev; t != NULL; t = t->prev)
    {
        if (name.empty() || CIsNode(t, name))
            return t;
    }

    return NULL;
}

/**
 * CGetAttribute ()
 * Get the value of an attribute.
 */
CString CGetAttribute(CNode *n, const CString &name)
{
    if (CHasAttribute(n, name))
    {
        xmlChar *prop = xmlGetProp(n, (const xmlChar *)name.c_str());
        CString attr = CString((const char *)prop);
        xmlFree(prop);
        return attr;
    }
    else
    {
        std::cerr << CIsNode(n) << " " << name << std::endl;
        throw CException("(CGetAttribute) Node does not have requested attribute.");
    }

    return CString("");
}

/**
 * CHasAttribute ()
 * Check that attribute is present.
 */
bool CHasAttribute(CNode *n, const CString &name)
{
    xmlAttr *attr;

    // Find the attribute
    attr = xmlHasProp(n, (const xmlChar *) name.c_str());

    // Check attribute
    if (attr == NULL)
        return false;

    // Attribute found
    return true;
}

/**
 * CAddAttribute ()
 * Add an attribute to the node.
 */
void CAddAttribute(CNode *n, const CString &name, const CString &value)
{
    // Check that attribute is already present
    if (CHasAttribute(n, name))
    {
        // Update existing attribute
        xmlSetProp(n, (const xmlChar *) name.c_str(), (const xmlChar *)value.c_str());
    }
    else
    {
        // Create new attribute
        xmlNewProp(n, (const xmlChar *) name.c_str(), (const xmlChar *) value.c_str());
    }
}

/**
 * CSetAttribute ()
 * Set or reset an attribute of the node.
 */
void CSetAttribute(CNode *n, const CString &name, const CString &value)
{
    xmlSetProp(n, (const xmlChar *) name.c_str(), (const xmlChar *) value.c_str());
}

/**
 * CRemoveAttribute ()
 * Remove attribute from node.
 */
void CRemoveAttribute(CNode *n, const CString &name)
{
    xmlAttr *prop;

    // Find the attribute
    prop = xmlHasProp(n, (const xmlChar *) name.c_str());

    if (prop == NULL)
        return;

    // Remove the attribute
    xmlRemoveProp(prop);
}

/**
 * CSetContent ()
 * Set content of node.
 */
void CSetContent(CNode *n, const CString &cont)
{
    xmlNodeSetContent(n, (const xmlChar *) cont.c_str());
}


/**
 * CGetNumberOfChildNodes ()
 * Return the number of child nodes with name 'name'.
 */
CId CGetNumberOfChildNodes(CNode *n, const CString &name)
{
    CId NumberOfChildNodes = 0;

    if (n != NULL)
    {
        for (CNode *t = n->children; t != NULL; t = t->next)
        {
            if (name.empty() || CIsNode(t, name))
            {
                NumberOfChildNodes++;
            }
        }
    }

    return NumberOfChildNodes;
}

void releaseDoc(CDoc *doc)
{
    xmlFreeDoc(doc);
}

/**
 * Create a procesing instruction.
 */
void CAddProcessingInstruction(CDoc *doc, const CString &pi, const CString &piArgs)
{
    xmlNodePtr newPI = xmlNewDocPI(doc, (xmlChar *)pi.c_str(), (xmlChar *)piArgs.c_str());
    xmlNodePtr rootNode = xmlDocGetRootElement(doc);
    xmlAddPrevSibling(rootNode, newPI);
}

/**
 * CSetRootNode ()
 * Set the document root node
 */
void CSetRootNode(CDoc *doc, CNode *rootNode)
{
    xmlDocSetRootElement(doc, rootNode);
}


/**
 * xmlCleanup()
 * call at the end to release xml related resources.
 */
void xmlCleanup()
{
    xmlCleanupParser();
}

#endif

