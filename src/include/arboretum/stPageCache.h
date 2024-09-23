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
* This file defines the class stDiskPageManager.
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
*/
#ifndef __STPAGECACHE_H
#define __STPAGECACHE_H

#include <stdexcept>

#include  <arboretum/stPage.h>

//==============================================================================
// stDiskPageManager
//------------------------------------------------------------------------------
/**
* This class defines the basic interface for all page caches used by page manager
* implementations.
*
* <p>A cache is not suposed to destroy, create, read or write pages because it 
* acts as an associative memory only. In other words, the hard work must be
* performed by the page manager.
*
* <p>Description of the usage is missing!!!
*
* @version 1.0
* @author Fabio Jun Takada Chino (chino@icmc.usp.br)
* @author Marcos Rodrigues Vieira (mrvieira@icmc.usp.br)
* @ingroup storage
* @warning This class will replace CNSCache in future versions of this library.
* @todo The definition of this class must be enhanced.
*/
class stPageCache{
   public:
      /**
      * Returns a page with a given page id. 
      *
      * @param pageID The given page id.
      * @return The desired page or NULL it the page could not be found.
      * @warning Due to optimization reasons, the returned instance will not
      * be removed from the cache. Be aware of that.
      */
      virtual stPage * Get(u_int32_t pageID) = 0;
      
      /**
      * Notifies the cache that a given page was modified. If the page is not
      * registered in the cache, this method does nothing.
      */
      virtual void SetModified(u_int32_t pageID) = 0;

      /**
      * Adds a new page into the cache. It may return the instance of the page
      * that must be removed from the cache.
      */
      virtual stPage * Add(stPage * page, bool & modified) = 0;
            
      /**
      * Removes a page from the cache. This method must be used by page managers
      * to flush all pages in the cache before its destruction.
      *
      * @retval A flag that tells if the returned page was modified since it was
      * read for the first time.
      * @return A page instance or null if this cache is empty.
      * @warning The order of the pages are undetermined.
      */
      virtual stPage * Pop(bool & modified) = 0;
};//end stPageCache

#endif //__STPAGECACHE_H
