/**********************************************************************************
 * Implement USBDevNode.h
 *********************************************************************************/

#include "USBDevNode.h"
#include <assert.h>

#define CM_SUCCESS(ret) assert(CR_SUCCESS == ret)

#define RETURN_IF_FAIL(ret)  \
do \
{ \
	if ( CR_SUCCESS != ret ) \
	{ \
		return false; \
	} \
}while(0)

using namespace SiS::Model;
using namespace SiS;

USBDevNode::USBDevNode() : m_root(0), m_parent(0)
{}

USBDevNode::~USBDevNode()
{
	for ( USBDevNodeList::iterator dn = m_childs.begin(); dn != m_childs.end(); dn++ )
	{
		if ( 0 != (*dn) )
		{
			delete (*dn);
			(*dn) = 0;
		}
	}
}

/*
USBDevNode::USBDevNode(const USBDevNode & _USBDevNode)
{}

const USBDevNode & USBDevNode::operator=(const USBDevNode & rhs)
{}
*/

void USBDevNode::setInstanceIDStr(AUChar * str)
{
	assert(0 != str);

	m_instIDStr = AUString(str);
}

void USBDevNode::setInstanceIDStr(AUString str)
{
	m_instIDStr = str;
}

AUString USBDevNode::instanceIDStr() const
{
	return m_instIDStr;
}

void USBDevNode::setInstanceID(DEVINST id)
{
	m_instID = id;
}

DEVINST USBDevNode::instanceID() const
{
	return m_instID;
}

void USBDevNode::setRoot(USBDevNode * node)
{
	m_root = node;
}

USBDevNode * USBDevNode::root()
{
	return m_root;
}

void USBDevNode::setParent(USBDevNode * node)
{
	m_parent = node;
}

USBDevNode * USBDevNode::parent()
{
	return m_parent;
}

void USBDevNode::setPID(const AUString & pid)
{
	m_pid = pid;
}

AUString USBDevNode::PID() const
{
	return m_pid;
}

void USBDevNode::setVID(const AUString & vid)
{
	m_vid = vid;
}

AUString USBDevNode::VID() const
{
	return m_vid;
}

void USBDevNode::addChild(USBDevNode * node)
{
	assert( 0 != node );
	m_childs.push_back(node);
}

USBDevNode * USBDevNode::child(const size_t idx)
{
	assert( idx < m_childs.size() );
	return m_childs[idx];
}

USBDevNode * USBDevNode::findChild(const AUChar * instIDStr)
{
	assert(0 != instIDStr);
	AUString target(instIDStr);
	return this->findChild(target);
}

USBDevNode * USBDevNode::findChild(const AUString instIDStr)
{
	for ( USBDevNodeList::iterator dn = m_childs.begin(); dn != m_childs.end(); dn++ )
	{
		if ( (*dn)->instanceIDStr() == instIDStr )
		{
			return (*dn);
		}
	}

	return 0;
}

size_t USBDevNode::numChild() const
{
	return m_childs.size();
}

USBDevNode * USBDevNode::buildTree(HDEVINFO hDevInfoSet, SP_DEVINFO_DATA devInfo)
{
	/* get instance id */
	DWORD _requiredSize = 0;
	DWORD requiredSize = 0;
	SetupDiGetDeviceInstanceId(hDevInfoSet, &devInfo, NULL, 0, &_requiredSize);
	requiredSize = _requiredSize;
	SetupDiGetDeviceInstanceId(hDevInfoSet, &devInfo, NULL, requiredSize, &_requiredSize);

	ULONG depth = 0;
	CONFIGRET retCode = CM_Get_Depth(&depth, devInfo.DevInst, 0);
	//printf("the depth is %d\n", depth);
	CM_SUCCESS(retCode);

	AUChar name[MAX_DEVICE_ID_LEN];
	retCode = CM_Get_Device_ID(devInfo.DevInst, name, MAX_DEVICE_ID_LEN, 0);
	AUString currentPid;
	AUString currentVid;

	if ( false == getPidVidFromInstanceId(name, currentVid, currentPid) )
	{
		return 0;
	}

	DEVINST currentDevInst = devInfo.DevInst;
	DEVINST parentDevInst;
	for ( unsigned int dIdx = 0; dIdx < depth; dIdx++ )
	{
		memset(name, 0, MAX_DEVICE_ID_LEN);
		retCode = CM_Get_Parent( &parentDevInst, currentDevInst, 0 );
		CM_SUCCESS(retCode);

		retCode = CM_Get_Device_ID(parentDevInst, name, MAX_DEVICE_ID_LEN, 0);
		CM_SUCCESS(retCode);

		AUString vid;
		AUString pid;
		if ( true == getPidVidFromInstanceId(name, vid, pid) )
		{
			if ( vid != currentVid || pid != currentPid )
			{
				break;
			}
		}
		else
		{
			/* there is no such vid or pid, this is must NOT a normal usb device */
			break;
		}
		currentDevInst = parentDevInst;
		//wprintf(L"%s\n", name);
	}

	/* current instance id is root of given device */
	USBDevNode * root = _buildTree(currentDevInst, 0, 0, false);

	return root;
}

bool USBDevNode::getPidVidFromInstanceId(AUChar * instIdStr, AUString & vid, AUString & pid)
{
	assert( 0 != instIdStr );
	AUString instId(instIdStr);

	/*
	static const AUString pidPrefix = "PID_";
	static const AUString vidPrefix = "VID_";
	*/
	static const AUString pidPrefix = AUTEXT(PID_);
	static const AUString vidPrefix = AUTEXT(VID_);
	static const size_t idLen = 4;

	int pidPos = instId.find(pidPrefix);
	if ( std::string::npos == pidPos )
	{
		//this->debugLog("parse location fail : " + rawStr);
		return false;
	}
	int vidPos = instId.find(vidPrefix);
	if ( std::string::npos == vidPos )
	{
		//this->debugLog("parse location fail : " + rawStr);
		return false;
	}

	pid = instId.substr( pidPos + pidPrefix.size(), idLen);
	vid = instId.substr( vidPos + vidPrefix.size(), idLen);

	return true;
}

/*---------------------------------------------------------------------------------
 * private
 *-------------------------------------------------------------------------------*/
//bool USBDevNode::_buildTree(const DEVINST currentDevInst, USBDevNode * currentNode, USBDevNode * root)
//bool * USBDevNode::_buildTree(const DEVINST currentDevInst, USBDevNode * parentNode, USBDevNode * root)
USBDevNode * USBDevNode::_buildTree(
		const DEVINST currentDevInst, USBDevNode * parentNode, USBDevNode * root, bool findSilbing)
{
	//assert( 0 != currentNode );

	USBDevNode * currentNode = new USBDevNode();
	if ( 0 == currentNode )
	{
		root = currentNode;
	}

	DEVINST _childIns;
	DEVINST childIns;
	CONFIGRET retCode = CM_Get_Child(&_childIns, currentDevInst, 0);
	if ( CR_SUCCESS == retCode )
	{
		/*
		do
		{
			childIns = _childIns;
			_buildTree(childIns, currentNode);
			safeCount--;
		}while( CR_SUCCESS == CM_Get_Sibling(&childIns, _childIns, 0) && safeCount > 0)
		*/
		childIns = _childIns;
		_buildTree(childIns, currentNode, root, true);
	}

	/* no more child */

	/*
	 * fill data here.
	 */
	currentNode->setInstanceID(currentDevInst);
	currentNode->setParent(parentNode);
	if ( 0 != parentNode )
	{
		parentNode->addChild(currentNode);
	}
	currentNode->setRoot(root);

	AUChar name[MAX_DEVICE_ID_LEN];
	retCode = CM_Get_Device_ID(currentDevInst, name, MAX_DEVICE_ID_LEN, 0);
	if ( CR_SUCCESS == retCode )
	{
		currentNode->setInstanceIDStr(name);
		AUString pid, vid;
		if ( true == getPidVidFromInstanceId(name, vid, pid) )
		{
			currentNode->setVID(vid);
			currentNode->setPID(pid);
		}
	}


	/* handle sibling */
	/*
	size_t safeCount = 1000;
	while( CR_SUCCESS == CM_Get_Sibling(&_childIns, currentDevInst, 0) && safeCount > 0 && true == findSilbing)
	{
		childIns = _childIns;
		_buildTree(childIns, parentNode, root, true);
	}
	*/
	if( CR_SUCCESS == CM_Get_Sibling(&_childIns, currentDevInst, 0) && true == findSilbing)
	{
		childIns = _childIns;
		_buildTree(childIns, parentNode, root, true);
	}

	return currentNode;
}
