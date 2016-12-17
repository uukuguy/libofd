#include <iostream>
#include <assert.h>
#include "OFDPackage.h"
#include "OFDDocument.h"
#include "OFDPage.h"
#include "utils/logger.h"

using namespace ofd;

void test_libofd(int argc, char *argv[]){
    OFDPackage package;
    if ( package.Open(argv[1]) ){
        //OFDDocument *document = package.GetOFDDocument(); 
        OFDDocumentPtr document = package.GetOFDDocument(); 

        LOG(DEBUG) << document->String();

        size_t n_pages = document->GetPagesCount();
        LOG(INFO) << "Loading " << n_pages << " pages.";

        for ( size_t i = 0 ; i < n_pages ; i++ ){
            TIMED_SCOPE(timerDrawPage, "Draw Page");

            //OFDPage *page = document->GetOFDPage(i);
            OFDPagePtr page = document->GetOFDPage(i);
            page->Open();

            VLOG(3) << page->String(); 
            LOG(INFO) << page->GetText();

            std::stringstream ss;
            ss << "Page" << (i + 1) << ".png";
            std::string png_filename = ss.str();
            page->RenderToPNGFile(png_filename);

            page->Close();
        }

        package.Close();
    } else {
        LOG(ERROR) << "Open package " << argv[1] << " failed.";
    }
}

#include <gflags/gflags.h>
DEFINE_int32(v, 0, "Logger level.");
int main(int argc, char *argv[]){
    TIMED_FUNC(timerMain);

    gflags::SetVersionString("1.0.0");
    gflags::SetUsageMessage("Usage: firsttest <pdffile>");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    Logger::Initialize(FLAGS_v);

    test_libofd(argc, argv);

    return 0;
}

