#include <string>

#include "PDFExtractor.h"
#include "logger.h"


const int DEFAULT_DPI = 72;

class PDFExtractor::InnerData {
public:
    InnerData(): m_pdfDoc(nullptr){
    }

    ~InnerData(){};

    PDFDoc *m_pdfDoc;

}; //class PDFExtractor::InnerData


template<typename T, typename ...Args>
std::unique_ptr<T> make_unique(Args&& ...args){
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// ======== PDFExtractor::PDFExtractor() ========
PDFExtractor::PDFExtractor()
    : OutputDev(), m_innerData(make_unique<PDFExtractor::InnerData>()){
}

// ======== PDFExtractor::~PDFExtractor() ========
PDFExtractor::~PDFExtractor(){
    Object::memCheck(stderr);
    gMemReport(std::stderr);
}

// ======== PDFExtractor::Process() ========
bool PDFExtractor::Process(const std::string &pdfFilename, const std::string &ownerPasswd, const std::string &userPasswd){
    bool ok = false;

    if ( OpenPDFFile(pdfFilename, ownerPasswd, userPasswd) ){

        PDFDoc *pdfDoc = m_innerData->m_pdfDoc;
        if ( pdfDoc != nullptr ){
            auto numPages = m_innerData->m_pdfDoc->getNumPages();
            for(auto i = 0; i <= numPages ; ++i) {
                std::cerr << "Preprocessing: " << i + 1 << "/" << numPages << '\r' << std::flush;

                Page *pdfPage = pdfDoc->getPage(i+1);
                if ( pdfPage != nullptr ){
                    PDFRectangle *mediaBox = pdfPage->getMediaBox();
                    LOG(DEBUG) << "mdeiaBox:(" << mediaBox->x1 << ", " << mediaBox->y1
                        << ", " << mediaBox->x2 << ", " << mediaBox->y2 << ")";

                    LOG(DEBUG) << "Start pdfDoc->displayPage() Page:" << i;

                    pdfPage->display(this, DEFAULT_DPI, DEFAULT_DPI,
                            0, 
                            0, //(!(param.use_cropbox)),
                            true,  // crop
                            false, // printing
                            nullptr, nullptr, nullptr, nullptr);

                } else {
                    LOG(WARNING) << "Page " << i << " is nullptr.";
                }
            }

            if(numPages >= 0)
                std::cerr << "Preprocessing: " << numPages << "/" << numPages;
            std::cerr << std::endl;


            ClosePDFFile();
            ok = true;
        } else {
            LOG(WARNING) << "pdfDoc is nullptr";
        }
    }

    return ok;
}

bool PDFExtractor::OpenPDFFile(const std::string &pdfFilename, const std::string &ownerPasswd, const std::string &userPasswd){
    bool ok = false;

    GooString fileName(pdfFilename.c_str());
    GooString *ownerPW = (ownerPasswd == "") ? nullptr: new GooString(ownerPasswd.c_str()); 
    GooString *userPW = (userPasswd == "") ? nullptr : new GooString(userPasswd.c_str());

    PDFDoc *pdfDoc = PDFDocFactory().createPDFDoc(fileName, ownerPW, userPW);
    if ( pdfDoc != nullptr ){
        if ( pdfDoc->isOk() ){
            if (pdfDoc->okToCopy()){
                size_t num_pages = pdfDoc->getNumPages();
                LOG(DEBUG) << "Total " << num_pages << " pages in pdf file: " << pdfFilename;
                //m_innerData->m_pdfDoc = std::move(pdfDoc);
                m_innerData->m_pdfDoc = pdfDoc;
                ok = true;
            } else {
                LOG(ERROR) << "PDF file is not okToCopy. " << pdfFilename;
            }
        } else {
            LOG(ERROR) << "Cann't read pdf file: " << pdfFilename;
        }

        if ( !ok ){
            delete pdfDoc;
            pdfDoc = nullptr;
        }
    } else {
        LOG(WARNING) << "pdfDoc == nullptr";
    }

    if ( ownerPW != nullptr ){
        delete ownerPW;
        ownerPW = nullptr;
    }
    if ( userPW != nullptr ){
        delete userPW;
        userPW = nullptr;
    }

    return ok;
}

void PDFExtractor::ClosePDFFile(){
}


void PDFExtractor::drawChar(GfxState *state, double x, double y,
        double dx, double dy,
        double originX, double originY,
        CharCode code, int nBytes, Unicode *u, int uLen){
    LOG(DEBUG) << "call drawChar()";
}

