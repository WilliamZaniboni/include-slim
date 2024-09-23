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
#ifndef __STMULTIPLEPAGEMANAGER_H
#define __STMULTIPLEPAGEMANAGER_H

#include <arboretum/stPageManager.h>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::to_string;
using std::exception;

template <class PageManager>
class stMultiplePageManager : public stPageManager {
public:
    stMultiplePageManager(const string &fileName,
                          u_int32_t pagesPerPM,
                          u_int32_t pageSize);

    stMultiplePageManager(const string &fileName,
                          u_int32_t pagesPerPM);

    virtual ~stMultiplePageManager();

    virtual bool IsEmpty();

    virtual stPage* GetHeaderPage();

    virtual stPage* GetPage(u_int32_t pageID);

    virtual void ReleasePage(stPage *page);

    virtual stPage* GetNewPage();

    virtual void WritePage(stPage *page);

    virtual void WriteHeaderPage(stPage *headerPage);

    virtual void DisposePage(stPage *page);

    virtual u_int32_t GetMinimumPageSize();

    virtual u_int32_t GetPageCount();

    virtual void ResetStatistics();

private:
    string fileName;

    u_int32_t pagesPerPM;

    u_int32_t pageCount;

    vector<PageManager*> PMs;

    u_int32_t GetCorrespondentPM(u_int32_t logicalPageID);

    u_int32_t GetActualPageID(u_int32_t logicalPageID);
};

#include "stMultiplePageManager-inl.h"

#endif
