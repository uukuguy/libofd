#include <iostream>
#include "OFDPackage.h"
#include "OFDDocument.h"
#include "OFDPage.h"
#include "logger.h"

using namespace ofd;

int main(int argc, char *argv[]){
    if ( argc <= 1 ){
        std::cout << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 0;
    }

    TIMED_FUNC(timerMain);

    Logger::Initialize(argc, argv);

    LOG(INFO) << "Start " << argv[0];

    OFDPackage package;
    if ( package.Open(argv[1]) ){
        OFDDocument *document = package.GetOFDDocument(); 
        LOG(DEBUG) << document->String();

        size_t n_pages = document->GetPagesCount();
        LOG(INFO) << "Loading " << n_pages << " pages.";

        for ( size_t i = 0 ; i < n_pages ; i++ ){
            TIMED_SCOPE(timerDrawPage, "Draw Page");

            OFDPage *page = document->GetOFDPage(i);
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

    LOG(INFO) << "Done.";

    return 0;
}
