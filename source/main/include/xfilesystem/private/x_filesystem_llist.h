#ifndef __XFILESYSTEM_LINKED_LIST_H__
#define __XFILESYSTEM_LINKED_LIST_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

// Includes
#include "xbase\x_types.h"
#include "xbase\x_debug.h"

namespace xcore
{
	namespace xfilesystem
	{
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// Linked List
		//
		template <class T> 
		class llist_item
		{
			llist_item*		mPrev;
			llist_item*		mNext;
			T				mObject;

		public:
			llist_item						( T xObject ) : mPrev(0), mNext(0), mObject(xObject)
			{
			}

			T				getItem			()											{ return mObject; }

			llist_item*		getPrev			()											{ return mPrev; }
			llist_item*		getNext			()											{ return mNext; }

			void			setPrev			( llist_item* pPrev )						{ mPrev = pPrev; }
			void			setNext			( llist_item* pNext )						{ mNext = pNext; }
		};

		template <class T> 
		class llist_item_ptr
		{
			llist_item_ptr*	mPrev;
			llist_item_ptr*	mNext;
			T*				mObject;

		public:

			llist_item_ptr					( ) : mPrev(0), mNext(0), mObject(NULL)
			{
			}

			llist_item_ptr					( T* pObject ) : mPrev(0), mNext(0), mObject(pObject)
			{
			}

			T*				getItem			()											{ return mObject; }
			void			setItem			( T* pObject )								{ mObject = pObject; }

			llist_item_ptr*getPrev			()											{ return mPrev; }
			llist_item_ptr*getNext			()											{ return mNext; }

			void			setPrev			( llist_item_ptr* pPrev )					{ mPrev = pPrev; }
			void			setNext			( llist_item_ptr* pNext )					{ mNext = pNext; }
		};

		template <class T>
		class llist
		{
		public:
							llist	();
							~llist	();

			// Add an entry to the start of the list
			void			addToHead		( T* pObject );
			T*				removeHead		( void );

			// Add an entry to the end of the list
			void			addToTail		( T* pObject );
			T*				removeTail		( void );

			// Add an entry before the specified entry
			void			addBefore		( T* pObject, T* pCurrent );

			// Add an entry after the specified entry
			void			addAfter		( T* pObject, T* pCurrent );

			// Remove the specified entry
			void			remove			( T* pObject );

			// Removes the last entry in the list
			T*				getHead			( void )								{ return mHead; };
			T*				getTail			( void )								{ return mTail; };
			T*				getNext			( T* pObject )							{ return pObject->getNext(); };
			T*				getPrev			( T* pObject )							{ return pObject->getPrev(); };

			T*				getByIndex		( xcore::s32 nIndex );
			xcore::s32		getLength		( void )								{ return mNumObjects; };

		private:
			T*				mHead;
			T*				mTail;
			xcore::s32		mNumObjects;
		};



		//-------------------
		// Template functions
		//-------------------

		template <class T> llist<T>::llist()
		{
			// Set the head & tail to NULL
			mHead			= NULL;
			mTail			= NULL;
			mNumObjects		= 0;
		}

		template <class T> llist<T>::~llist()
		{
		}

		template <class T> void llist<T>::addToHead(T* pObject)
		{
			if(mHead)
			{
				pObject->setPrev(NULL);
				pObject->setNext(mHead);

				mHead->setPrev(pObject);

				mHead	= pObject;
			}
			else
			{
				pObject->setPrev(NULL);
				pObject->setNext(NULL);

				mHead	= pObject;
				mTail	= pObject;
			}

			mNumObjects++;
		}

		template <class T> T* llist<T>::removeHead(void)
		{
			T* pOldHead = NULL;
			if (mHead)
			{
				pOldHead = mHead;
				if(mHead->getNext())
				{
					mHead->getNext()->setPrev(NULL);
					mHead = mHead->getNext();
				}
				else
				{
					mHead	= NULL;
					mTail	= NULL;
				}

				mNumObjects--;

				pOldHead->setPrev(NULL);
				pOldHead->setNext(NULL);
			}
			return pOldHead;
		}

		template <class T> void llist<T>::addToTail(T* pObject)
		{
			if(mTail)
			{
				pObject->setPrev(mTail);
				pObject->setNext(NULL);

				mTail->setNext(pObject);

				mTail	= pObject;
			}
			else
			{
				pObject->setPrev(NULL);
				pObject->setNext(NULL);

				mHead	= pObject;
				mTail	= pObject;
			}

			mNumObjects++;
		}

		template <class T> T* llist<T>::removeTail(void)
		{
			T*	pOldTail = NULL;
			if(mTail)
			{
				pOldTail = mTail;

				if(mTail->getPrev())
				{
					mTail->getPrev()->setNext(NULL);
					mTail = mTail->getPrev();
				}
				else
				{
					mHead	= NULL;
					mTail	= NULL;
				}

				mNumObjects--;

				pOldTail->setPrev(NULL);
				pOldTail->setNext(NULL);
			}
			return pOldHead;
		}

		template <class T> void llist<T>::addBefore(T* pObject, T* pCurrent)
		{
			ASSERT(pObject);
			ASSERT(pCurrent);

			if(pCurrent->getPrev())
			{
				pCurrent->getPrev()->setNext(pObject);
				pObject->setPrev(pCurrent->getPrev());

				pCurrent->setPrev(pObject);
				pObject->setNext(pCurrent);

				++mNumObjects;
			}
			else
			{
				ASSERT(pCurrent == mHead);

				addToHead(pObject);
			}
		}

		template <class T> void llist<T>::addAfter(T* pObject, T* pCurrent)
		{
			ASSERT(pObject);
			ASSERT(pCurrent);

			if(pCurrent->getNext())
			{
				pCurrent->getNext()->setPrev(pObject);
				pObject->setNext(pCurrent->getNext());

				pCurrent->setNext(pObject);
				pObject->setPrev(pCurrent);

				++mNumObjects;
			}
			else
			{
				ASSERT(pCurrent == mTail);

				addToTail(pObject);
			}
		}

		template <class T> void llist<T>::remove(T* pObject)
		{
			ASSERT(pObject);

			if(pObject->getNext())
			{
				pObject->getNext()->setPrev(pObject->getPrev());
			}
			else
			{
				mTail = pObject->getPrev();
			}

			if(pObject->getPrev())
			{
				pObject->getPrev()->setNext(pObject->getNext());
			}
			else
			{
				mHead = pObject->getNext();
			}

			pObject->setNext(NULL);
			pObject->setPrev(NULL);

			--mNumObjects;
		}

		template <class T> T* llist<T>::getByIndex(xcore::s32 nIndex)
		{
			ASSERT(nIndex < mNumObjects);

			int32	nCount	= 0;
			T*		pObject	= mHead;

			while(nCount < nIndex)
			{
				pObject	= pObject->getNext();
				nCount++;
			}

			return pObject;
		}
	}
};

#endif // __XFILESYSTEM_LINKED_LIST_H__