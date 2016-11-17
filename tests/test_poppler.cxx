
#include <string>

// -------- poppler --------
#include <poppler-config.h>
#include <goo/GooString.h>

#include <Object.h>
#include <PDFDoc.h>
#include <PDFDocFactory.h>
#include <GlobalParams.h>

#include "logger.h"

struct Param
{
    // pages
    int first_page, last_page;

    // dimensions
    double zoom;
    double fit_width, fit_height;
    int use_cropbox;
    double h_dpi, v_dpi;

    // output
    int embed_css;
    int embed_font;
    int embed_image;
    int embed_javascript;
    int embed_outline;
    int split_pages;
    std::string dest_dir;
    std::string css_filename;
    std::string page_filename;
    std::string outline_filename;
    int process_nontext;
    int process_outline;
    int process_annotation;
    int process_form;
    int correct_text_visibility;
    int printing;
    int fallback;
    int tmp_file_size_limit;

    // fonts
    int embed_external_font;
    std::string font_format;
    int decompose_ligature;
    int auto_hint;
    std::string external_hint_tool;
    int stretch_narrow_glyph;
    int squeeze_wide_glyph;
    int override_fstype;
    int process_type3;

    // text
    double h_eps, v_eps;
    double space_threshold;
    double font_size_multiplier;
    int space_as_offset;
    int tounicode;
    int optimize_text;

    // background image
    std::string bg_format;
    int svg_node_count_limit;
    int svg_embed_bitmap;

    // encryption
    std::string owner_password, user_password;
    int no_drm;

    // misc.
    int clean_tmp;
    std::string data_dir;
    std::string poppler_data_dir;
    std::string tmp_dir;
    int debug;
    int proof;

    std::string input_filename, output_filename;
};

#include "PDFExtractor.h"

void test_poppler(int argc, char *argv[]){
    //PDFDoc *pdfdoc = nullptr;
    //GooString *ownerPW=(param.owner_password == "") ? (nullptr) : (new GooString(param.owner_password.c_str()));

    std::string pdfFilename = argv[1];
    std::string ownerPasswd = "";
    std::string userPasswd = "";
    LOG(DEBUG) << "Try to open pdf file: " << pdfFilename;

    PDFExtractor pdfExtractor;
    pdfExtractor.Process(pdfFilename, ownerPasswd, userPasswd);


    //GError* gerror  = nullptr;
    //PopplerDocument* poppler_document = poppler_document_new_from_file(pdf_filename.c_str(),
      //nullptr, &gerror);
    //if ( poppler_document != nullptr ){
        //size_t num_pages = poppler_document_get_n_pages(poppler_document);
        //LOG(DEBUG) << "Total " << num_pages << " pages in pdf file: " << pdf_filename;

        //g_object_unref(poppler_document);

    //} else {
        //if ( gerror != nullptr && gerror->code == POPPLER_ERROR_ENCRYPTED){
            //LOG(WARNING) << "Open pdf file failed. ERROR_INVALID_PASSWORD" << pdf_filename;
        //} else {
            //LOG(ERROR) << "Open pdf file failed. " << pdf_filename;
        //}
            
    //}

    //GooString fileName(pdf_filename.c_str());
    //GooString *ownerPW = (owner_passwd == "") ? nullptr: new GooString(owner_passwd.c_str(), owner_passwd.length());
    //GooString *userPW = (user_passwd == "") ? nullptr : new GooString(user_passwd.c_str(), user_passwd.length());

    //PDFDoc *pdfdoc = PDFDocFactory().createPDFDoc(fileName, ownerPW, userPW);
    //if ( pdfdoc != nullptr ){
        //if ( pdfdoc->isOk() ){
            //if (pdfdoc->okToCopy()){
                //size_t num_pages = pdfdoc->getNumPages();
                //LOG(DEBUG) << "Total " << num_pages << " pages in pdf file: " << pdf_filename;



            //} else {
                //LOG(ERROR) << "PDF file is not okToCopy. " << pdf_filename;
            //}
        //} else {
            //LOG(ERROR) << "Cann't read pdf file: " << pdf_filename;
        //}

        //delete pdfdoc;
        //pdfdoc = nullptr;
    //} else {
        //LOG(WARNING) << "pdfdoc == nullptr";
    //}

    //if ( ownerPW != nullptr ){
        //delete ownerPW;
        //ownerPW = nullptr;
    //}
    //if ( userPW != nullptr ){
        //delete userPW;
        //userPW = nullptr;
    //}

    //Object::memCheck(stderr);
    //gMemReport(std::stderr);
}

