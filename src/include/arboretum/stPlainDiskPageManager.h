/* Copyright 2003-2017 GBDI-ICMC-USP <caetano@icmc.usp.br>
* 
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* 
* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* 
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* 
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**
* @file
*
* This file defines the class stPlainDiskPageManager.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/
#ifndef __STPLAINDISKPAGEMANAGER_H
#define __STPLAINDISKPAGEMANAGER_H

#include <stdexcept>

#include <arboretum/stPageManager.h>
#include <arboretum/stUtil.h>
#include <arboretum/stCommonIO.h>

//==============================================================================
// stPlainDiskPageManager
//------------------------------------------------------------------------------
/**
* This class implements a page manager capable to read and write disk pages. It
* was designed to provide a fair base for algorithm comparisons and is not suposed
* to be used by users which seek for performance.
*
* <p>The implementation is based on the standard I/O system calls. All read/write
* operations are performed without chaching pages. As an additional feature, it
* is possible to disable the system I/O cache in some operational systems.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @see stPageManager
* @see stDiskPageManager
* @see stMemoryPageManager
* @ingroup storage
*/
class stPlainDiskPageManager: public stPageManager{
   public:
      /**
      * Creates a new instance of this class. This constructor will create a new
      * file with the given name.
      *
      * @param fName The file name.
      * @param pagesize Size of each page in file. This value must be larger
      * or equal than 64.
      * @exception std::logic_error If the file can not be created.
      */
      stPlainDiskPageManager(const char * fName, u_int32_t pagesize);

      /**
      * Creates a new instance of this class. This constructor will open an
      * existing file.
      *
      * @param fName The file name.
      * @exception std::logic_error If the file can not be opened or the file is
      * not a valid disk page manager file.
      */
      stPlainDiskPageManager(const char * fName);
      
      /**
      * Disposes this page and free all allocated resources.
      */
      virtual ~stPlainDiskPageManager();

      /**
      * This method will checks if this page manager is empty.
      * If this method returns true, the stSlimTree will create a
      * new tree otherwise it will continue to use the existing tree.
      *
      * @return True if the page manager is empty or false otherwise.
      */
      virtual bool IsEmpty();

      /**
      * Returns the header page. This method will return a special
      * page that will be used by SlimTree to write some information
      * about the tree itself.
      *
      * <P>Since this page must always exist, this method will always
      * return a valid page for reading/writing. Use WritePage() to
      * write this page.
      *
      * <P>The returning instance of stPage will be locked to prevent
      * its reuse by this page manager. Use ReleasePage() to unlock
      * this instance.
      *
      * @return The header page.
      * @see WritePage()
      * @see ReleasePage()
      * @note This method performs a true read operation even if this
      * procedure is not required (just to compute read time).
      */
      virtual stPage * GetHeaderPage();

      /**
      * Returns the page with the given page ID. This method will
      * return a valid page for reading/writing. Use WritePage() to
      * write this page.
      *
      * <P>The returning instance of stPage will be locked to prevent
      * its reuse by this page manager. Use ReleasePage() to unlock
      * this instance.
      *
      * @param pageid The desired page id.
      * @return The page or NULL for an invalid page ID.
      * @see WritePage()
      * @see ReleasePage()
      */
      virtual stPage * GetPage(u_int32_t pageid);

      /**
      * Releases this instace for reuse by this page manager.
      * Since some implementations of page manager will reuse
      * instances of stPage to avoid unnecessary resource
      * reallocations, each page instance must be locked until
      * it becomes unecessary.
      *
      * <P>The stSlimTree will always call this method when a
      * stPage instance will not be used in a near future.
      *
      * @param page The locked page.
      * @see GetPage()
      * @see GetHeaderPage()
      */
      virtual void ReleasePage(stPage * page);

      /**
      * Allocates a new page for use. As GetPage() and
      * GetHeaderPage(), the returning instance will be
      * locked to prevent reuse by this page manager.
      *
      * <P>To dispose this page (make it free), use DisposePage().
      *
      * @return A new page or NULL for errors.
      * @see ReleasePage()
      * @see WritePage()
      * @see DisposePage()
      */
      virtual stPage * GetNewPage();

      /**
      * Writes the given page to the disk. This method
      * will write the page but will not release it. Use
      * ReleasePage() to do it.
      *
      * @param page The page to be written.
      * @see ReleasePage()
      */
      virtual void WritePage(stPage * page);

      /**
      * Writes the header page to the disk.
      *
      * @see ReleasePage()
      * @see WritePage()
      * @see GetHeaderPageSize()
      */
      virtual void WriteHeaderPage(stPage * headerpage);

      /**
      * Disposes the given page. This method will make the page
      * available (not allocated) for the next calls of GetNewPage().
      *
      * <P>Since this page will not be used anymore, this method will
      * release the lock for this page instance.
      *
      * @param page The page to be disposed.
      * @see GetNewPage()
      */
      virtual void DisposePage(stPage * page);

      /**
      * Returns the minimum size of a page. The size of the header page is
      * always ignored since it may be smaller than others.
      */ 
      virtual u_int32_t GetMinimumPageSize(){
         return header->PageSize;
      }//end GetMinimumPageSize

      /**
      * Returns the number of pages.
      */
      virtual u_int32_t GetPageCount() {
         return header->PageCount;
      }

      /**
      * This method allows users to enable/disable system I/O cache in certain
      * OS. If this operation is not supported, this implementation does nothing.
      *
      * @param enabled State of the system cache.
      */
      void SetSystemCache(bool enabled){
         // Nothing to do.. at least for now.
      }//end SetSystemCache
      
   private:
      #pragma pack(1)
      /**
      * The header of the file.
      */
      struct tHeader{
         /**
         * Magic header. Always "DPM1".
         */
         char Magic[4];
         
         /**
         * Size of each page in bytes.
         */
         u_int32_t PageSize;
         
         /**
         * Number of pages allocated including deleted ones and the header pages.
         * In other words, it is the id of last allocated page.
         */
         u_int32_t PageCount;
         
         /**
         * Number of used pages.
         */
         u_int32_t UsedPages;

         /**
         * The page ID of the first available page. It is used
         * to manage the
         */         
         u_int32_t Available;
      };//end tHeader
      #pragma pack()
      
      /**
      * Type of the instance cache used by this disk page manager.
      */
      typedef stInstanceCache <stPage, stPageAllocator> stPageInstanceCache;

      /**
      * The page instance cache used by this disk page manager. The header
      * page will not use the cache because it has a different size.
      */
      stPageInstanceCache * pageInstanceCache;
      
      /**
      * File descriptor.
      */
      int fd;
      
      /**
      * The header of this instance. It points to the headerPage's
      * internal buffer.
      */
      tHeader * header;

      /**
      * Header page cache. To make the comparison fair, this cache
      * is not used to avoid read/write operations. It is used to keep
      * the page manager's file header loaded.
      */      
      stLockablePage * headerPage;

      /**
      * Creates the header for an empty file.
      *
      * @param header the pointer to the header.
      * @param pagesize the size of the page.
      */
      void NewHeader(tHeader * header, u_int32_t pagesize);
      
      /**
      * Validates a header.
      *
      * @param header The header.
      * @return True for a valid header of false otherwise.
      */
      bool IsValidHeader(tHeader * header);
      
      /**
      * Converts a page ID to the file offset.
      *
      * @param pageid The page id.
      * @return The offset of the given page id.
      */
      u_int32_t PageID2Offset(u_int32_t pageid){
         return pageid * header->PageSize;
      }//end PageID2Offset
      
};//end stPlainDiskPageManager

#endif //__STPLAINDISKPAGEMANAGER_H
