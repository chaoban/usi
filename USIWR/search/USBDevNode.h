/******************************************************************************//**
 *  @file USBDevNode.h
 *  @class USBDevNode
 *  @brief A usb device node.
 * 
 *  We use a tree sturecture to represent a composite usb device so that we can
 *  search target interface, class or even sub-class quickly.
 *  
 *  @author CY Fan
 *  @date 2016-12-09, 10:48:51
 **********************************************************************************/
#ifndef USBDEVNODE_H_
#define USBDEVNODE_H_

#include "AUString.h"
#include <vector>

/* for hid device */
#include <windows.h>

extern "C" {
#include "cfgmgr32.h"
#include "setupApi.h"
}


namespace SiS
{
namespace Model // need refine this namespace
{

class USBDevNode;
typedef std::vector<USBDevNode*> USBDevNodeList;

class USBDevNode
{
    public:
	    USBDevNode();
	    ~USBDevNode();

		void setInstanceIDStr(AUChar * str);
		void setInstanceIDStr(AUString str);
		AUString instanceIDStr() const;

		void setInstanceID(DEVINST id);
		DEVINST instanceID() const;

		void setRoot(USBDevNode * node);
		USBDevNode * root();

		void setParent(USBDevNode * node);
		USBDevNode * parent();

		void setPID(const AUString & pid);
		AUString PID() const;

		void setVID(const AUString & vid);
		AUString VID() const;

		void addChild(USBDevNode * node);
		USBDevNode * child(const size_t idx);
		USBDevNode * findChild(const AUChar * instIDStr);
		USBDevNode * findChild(const AUString instIDStr);
		size_t numChild() const;

		static USBDevNode * buildTree(HDEVINFO hDevInfoSet, SP_DEVINFO_DATA DevInfo);
		static bool getPidVidFromInstanceId(AUChar * instIdStr, AUString & vid, AUString & pid);

		/*
        USBDevNode(const USBDevNode & _USBDevNode);
        const USBDevNode & operator=(const USBDevNode & rhs);
		*/
	private:
		//static bool _buildTree(const DEVINST currentDevInst, USBDevNode * currentNode, USBDevNode * root);
		static USBDevNode * _buildTree(
				const DEVINST currentDevInst, 
				USBDevNode * parentNode, 
				USBDevNode * root, 
				bool findSilbing);

	private:
		AUString m_instIDStr;
		DEVINST m_instID;
		USBDevNodeList m_childs;
		USBDevNode * m_root;
		USBDevNode * m_parent;
		AUString m_pid;
		AUString m_vid;
};

}; // Model
}; // SiS

#endif
