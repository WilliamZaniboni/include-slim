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
* This file defines the stMemoryPageManager.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @author Enzo Seraphim (seraphim@icmc.usp.br)
* @todo Review of documentation.
*/
#ifndef __SLIMTREE_STMEMORYPAGEMANAGER_H
#define __SLIMTREE_STMEMORYPAGEMANAGER_H

#include <arboretum/stPageManager.h>
#include <arboretum/stPage.h>

// Include exception class
#include <vector>
#include <stack>

/**
* This class implements a stPageManager that holds the pages in the main memory.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @author Enzo Seraphim (seraphim@icmc.usp.br)
* @see stDiskPageMamanger
* @todo Documentation review.
* @ingroup storage
*/
class stMemoryPageManager: public stPageManager{
   public:
      /**
      * Creates a new stMemoryPageManager.
      *
      * @param pagesize The size of each page.
      */
      stMemoryPageManager(u_int32_t pagesize);

      /**
      * Dispose this page and all allocated resoureces.
      */
      ~stMemoryPageManager();

      /**
      * Returns the size of each page.
      */
      u_int32_t GetPageSize(){

         return PageSize;
      }//end GetPageSize

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
         return PageSize;
      }//end GetMinimumPageSize

      /**
      * Returns the number of pages.
      */
      virtual u_int32_t GetPageCount() {
         return Pages.size();
      }
   private:
      /**
      * Size of each page.
      */
      u_int32_t PageSize;

      /**
      * Vector that will hold the pages in memory (a container).
      */
      std::vector <stPage *> Pages;

      /**
      * The stack of free pages.
      */
      std::stack <u_int32_t> FreePages;
};//end stMemoryPageManager


#endif //__SLIMTREE_STMEMORYPAGEMANAGER_H
