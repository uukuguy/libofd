#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cairo/cairo.h>
#include <assert.h>
#include "ofd/Package.h"
#include "ofd/Document.h"
#include "ofd/Page.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "OFDCairoRender.h"

using namespace ofd;
double g_resolutionX = 144.0;
double g_resolutionY = 144.0;
double ZOOM_STEP = 0.1;
double ZOOM_BASE = exp(1.0);
double X_STEP = 10;
double Y_STEP = 10;

// -------- create_image_surface() --------
SDL_Surface *create_image_surface(int width, int height, int bpp){
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000; 
    gmask = 0x00ff0000; 
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000; 
    amask = 0xff000000; 
#endif

    SDL_Surface *imageSurface = SDL_CreateRGBSurface(0, width, height, bpp, 
            rmask, gmask, bmask, amask);
    if ( imageSurface == nullptr ){
        LOG(ERROR) << "SDL_CreateRGBSurface() failed. " << SDL_GetError();
    }

    return imageSurface;
}

// -------- create_cairo_surface_from_sdl_surface() --------
cairo_surface_t *create_cairo_surface_from_sdl_surface(SDL_Surface *sdl_surface){
    cairo_surface_t *cairo_surface = nullptr;

    cairo_surface = cairo_image_surface_create_for_data(
            (unsigned char*)sdl_surface->pixels,
            CAIRO_FORMAT_ARGB32,
            sdl_surface->w,
            sdl_surface->h,
            sdl_surface->pitch
            );

    return cairo_surface;
}

// **************** class SDLApp ****************
class SDLApp {
public:
    SDLApp(const std::string &title, double screenWidth, double screenHeight, int screenBPP);
    virtual ~SDLApp();

    void Execute();

protected:
    virtual void OnEvent(SDL_Event event, bool &done);
    virtual void OnRender(cairo_surface_t *surface);

    void RenderScreen() const;
    void RefreshScreen(uint64_t color) const;
    void ToggleFullScreen();

private:
    std::string m_title;
    bool m_fullscreen;

protected:
    double m_screenWidth;
    double m_screenHeight;

private:
    int m_screenBPP;

    SDL_Window *m_mainWindow;
    SDL_Renderer *m_screenRenderer;

    bool init();
    void cleanup();
    void Loop();

protected:
    SDL_Surface *m_imageSurface;
    double m_pixelX;
    double m_pixelY;
    double m_scaling;
    double m_origPixelX;
    double m_origPixelY;
    double m_origScaling;
    double m_zoomFactor;
    bool   m_bHelp;

}; // class SDLApp

// ======== SDLApp::SDLApp() ========
SDLApp::SDLApp(const std::string &title, double screenWidth, double screenHeight, int screenBPP)
    : m_title(title), m_fullscreen(false), 
    m_screenWidth(screenWidth), m_screenHeight(screenHeight), m_screenBPP(screenBPP),
    m_mainWindow(nullptr), m_screenRenderer(nullptr), m_imageSurface(nullptr),
    m_pixelX(0.0), m_pixelY(0.0), m_scaling(1.0), 
    m_origPixelX(-1), m_origPixelY(-1), m_origScaling(-1), 
    m_zoomFactor(ZOOM_BASE), m_bHelp(false){
    init();
}

// ======== SDLApp::~SDLApp() ========
SDLApp::~SDLApp(){
    cleanup();
}


bool SDLApp::init(){
    int screenFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE; // SDL_WINDOW_BORDERLESS
    m_mainWindow = SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_screenWidth, m_screenHeight, screenFlags);
    if ( m_mainWindow == nullptr ){
        LOG(ERROR) << "SDL_CreateWindow() failed. " << SDL_GetError();
        return false;
    }

    m_screenRenderer = SDL_CreateRenderer(m_mainWindow, -1, SDL_RENDERER_ACCELERATED);// | SDL_RENDERER_PRESENTVSYNC);
    if ( m_screenRenderer == nullptr ){
        LOG(ERROR) << "SDL_CreateRenderer() failed. " << SDL_GetError();
        return false;
    }

    if ( !(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) ){
        LOG(ERROR) << "IMG_Init() failed. " << SDL_GetError();
        return false;
    }

    m_imageSurface = create_image_surface(m_screenWidth, m_screenHeight, m_screenBPP);
    if ( m_imageSurface == nullptr ){
        LOG(ERROR) << "create_image_surface() failed. " << SDL_GetError();
        return false;
    }

    return true;
}

void SDLApp::cleanup(){
    IMG_Quit();

    if ( m_imageSurface != nullptr ){
        SDL_FreeSurface(m_imageSurface);
        m_imageSurface = nullptr;
    }
    if ( m_screenRenderer != nullptr ){
        SDL_DestroyRenderer(m_screenRenderer);
        m_screenRenderer = nullptr;
    }
    if ( m_mainWindow != nullptr ){
        SDL_DestroyWindow(m_mainWindow);
        m_mainWindow = nullptr;
    }

    SDL_Quit();
}

void SDLApp::ToggleFullScreen(){
    if (m_fullscreen){
        SDL_SetWindowFullscreen(m_mainWindow, SDL_FALSE);
        m_fullscreen = false;
    } else {
        SDL_SetWindowFullscreen(m_mainWindow, SDL_TRUE);
        m_fullscreen = true;
    }
}

// ======== SDLApp::Execute() ========
void SDLApp::Execute(){
    bool done = false;
    while ( !done ){
        SDL_Event event;
        while ( SDL_PollEvent(&event) != 0 ){
            if ( event.type == SDL_QUIT ){
                done = true;
            }
            switch (event.type) {
            case SDL_KEYDOWN:
                if ( event.key.keysym.sym == SDLK_ESCAPE){
                    done = true;
                } else if ( event.key.keysym.sym == SDLK_q){
                    //SDL_GetModState() & KMOD_CTRL
                    //inputText = SDL_GetClipboardText();
                    done = true;
                } else if ( event.key.keysym.sym == SDLK_h ){
                    // Move window left
                    m_pixelX += X_STEP;
                } else if ( event.key.keysym.sym == SDLK_l ){
                    // Move window right
                    m_pixelX -= X_STEP;
                } else if ( event.key.keysym.sym == SDLK_k ){
                    // Move window up
                    m_pixelY += Y_STEP;
                } else if ( event.key.keysym.sym == SDLK_j ){
                    // Move window down 
                    m_pixelY -= Y_STEP;
                } else if ( event.key.keysym.sym == SDLK_i ){
                    // Zoom In
                    m_zoomFactor += ZOOM_STEP;
                } else if ( event.key.keysym.sym == SDLK_o ){
                    // Zoom Out
                    m_zoomFactor -= ZOOM_STEP;
                } else if ( event.key.keysym.sym == SDLK_f ){
                    // Zoom to Fit 
                    m_zoomFactor = ZOOM_BASE;
                    m_pixelX = 0;
                    m_pixelY = 0;
                } else if ( event.key.keysym.sym == SDLK_h ){
                    // Help 
                    m_bHelp = !m_bHelp;
                }
                break;
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                }
                break;
            case SDL_WINDOWEVENT:
                switch ( event.window.event ){
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    m_screenWidth = event.window.data1;
                    m_screenHeight = event.window.data2;

                    //SDL_DestroyRenderer(m_screenRenderer);
                    //m_screenRenderer = SDL_CreateRenderer(m_mainWindow, -1, SDL_RENDERER_ACCELERATED); // | SDL_RENDERER_PRESENTVSYNC);
                    //if ( m_screenRenderer == nullptr ){
                        //LOG(ERROR) << "SDL_CreateRenderer() failed. " << SDL_GetError();
                        // done = true;
                    //} else {

                    SDL_FreeSurface(m_imageSurface);
                    m_imageSurface = create_image_surface(m_screenWidth, m_screenHeight, m_screenBPP);
                    SDL_RenderPresent(m_screenRenderer);
                    // }
                    break;
                case SDL_WINDOWEVENT_EXPOSED:
                    SDL_RenderPresent(m_screenRenderer);
                    break;

                //case SDL_WINDOWEVENT_ENTER:
                    //break;
                //case SDL_WINDOWEVENT_LEAVE:
                    //break;
                //case SDL_WINDOWEVENT_FOCUS_GAINED:
                    //break;
                //case SDL_WINDOWEVENT_FOCUS_LOST:
                    //break;
                //case SDL_WINDOWEVENT_MINIMIZED:
                    //break;
                //case SDL_WINDOWEVENT_MAXIMIZED:
                    //break;
                //case SDL_WINDOWEVENT_RESTORED:
                    //break;
                };
                break;
            default:
                break;
            };
            OnEvent(event, done);
        };

        Loop();

        RenderScreen();

        SDL_Delay(1); // Breath
    };
}

// ======== SDLApp::RenderScreen() ========
void SDLApp::RenderScreen() const {
    // Render(imageSurface)
    //SDL_Texture *backgroundTexture = SDL_CreateTexture(m_screenRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, m_screenWidth, m_screenHeight);
    //if ( backgroundTexture == nullptr ){
        //LOG(ERROR) << "SDL_CreateTexture() failed. " << SDL_GetError();
        //return -1;
    //}
    //SDL_UpdateTexture(backgroundTexture, NULL, m_imageSurface->pixels, m_imageSurface->pitch);

    SDL_Texture *backgroundTexture = SDL_CreateTextureFromSurface(m_screenRenderer, m_imageSurface);

    // Clear screen
    SDL_SetRenderDrawColor(m_screenRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(m_screenRenderer);
    // Render texture to screen
    SDL_Rect destRect{0, 0, (int)m_screenWidth, (int)m_screenHeight};
    SDL_RenderCopy(m_screenRenderer, backgroundTexture, NULL, &destRect);

    // Update screen
    SDL_RenderPresent(m_screenRenderer);

    SDL_DestroyTexture(backgroundTexture);
}

void SDLApp::OnEvent(SDL_Event event, bool &done){
}

void SDLApp::OnRender(cairo_surface_t *surface){
}

// ======== SDLApp::RefreshScreen() ========
void SDLApp::RefreshScreen(uint64_t color) const{
    SDL_FillRect(m_imageSurface, NULL, color);
}

// ======== SDLApp::Loop() ========
void SDLApp::Loop(){
    RefreshScreen(0xFFFFFF);

    cairo_surface_t *cairoSurface = create_cairo_surface_from_sdl_surface(m_imageSurface);
    if ( cairoSurface == nullptr ){
        LOG(ERROR) << "create_cairo_surface_from_sdl_surface() failed.";
        return;
    }

    OnRender(cairoSurface);

    cairo_surface_destroy(cairoSurface);
}

// **************** class MySDLApp ****************
class MySDLApp : public SDLApp {
public:
    MySDLApp(const std::string &title, double screenWidth, double screenHeight, int screenBPP, DocumentPtr document);
    virtual ~MySDLApp();

    virtual void OnEvent(SDL_Event event, bool &done) override;
    virtual void OnRender(cairo_surface_t *surface) override;

    DocumentPtr m_document;
    size_t m_pageIndex;

    std::unique_ptr<OFDCairoRender> m_cairoRender;

}; // class MySDLApp


// ======== MySDLApp::MySDLApp() ========
MySDLApp::MySDLApp(const std::string &title, double screenWidth, double screenHeight, int screenBPP, DocumentPtr document) : 
    SDLApp(title, screenWidth, screenHeight, screenBPP),
    m_document(document), m_pageIndex(0){

    m_cairoRender = utils::make_unique<OFDCairoRender>(m_screenWidth, m_screenHeight, g_resolutionX, g_resolutionY);
}

// ======== MySDLApp::~MySDLApp() ========
MySDLApp::~MySDLApp(){
}

// ======== MySDLApp::OnEvent() ========
void MySDLApp::OnEvent(SDL_Event event, bool &done){
    switch (event.type) {
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_UP) {
            if ( m_document != nullptr ){
                size_t totalPages = m_document->GetNumPages();
                if ( totalPages > 0 ){
                    if ( m_pageIndex == 0 ){
                        m_pageIndex = totalPages - 1;
                    } else {
                        m_pageIndex--;
                    }
                }
                LOG(DEBUG) << "Page " << m_pageIndex << "/" << totalPages;
            }
            break;
        } else if (event.key.keysym.sym == SDLK_DOWN) {
            if ( m_document != nullptr ){
                size_t totalPages = m_document->GetNumPages();
                if ( totalPages > 0 ){
                    if ( m_pageIndex < totalPages - 1 ){
                        m_pageIndex++;
                    } else {
                        m_pageIndex = 0;
                    }
                }
                LOG(DEBUG) << "Page " << m_pageIndex << "/" << totalPages;
            }
            break;
        } else if (event.key.keysym.sym == SDLK_RETURN) {
            ToggleFullScreen();
            break;
        }
        break;
    };
}

// ======== MySDLApp::OnRender() ========
void MySDLApp::OnRender(cairo_surface_t *surface){
    //if ( m_document != nullptr ){
    if ( m_document != nullptr && m_cairoRender != nullptr ){
        size_t totalPages = m_document->GetNumPages();
        if ( totalPages > 0 ){
            PagePtr page = m_document->GetPage(m_pageIndex);
            if ( page->Open() ){
                //std::unique_ptr<OFDCairoRender> m_cairoRender(new OFDCairoRender(m_screenWidth, m_screenHeight, g_resolutionX, g_resolutionY));

                /***
                 * drawParams {pixelX, pixelY, scaling}
                 * (pixelX, pixelY) 显示窗口左上角坐标与页面原点（左上角）的像素偏移。
                 * scaling - 缩放比例
                 *
                 ***/
                double pixelX = m_pixelX;
                double pixelY = m_pixelY;
                double scaling = m_scaling;

                double delta = m_zoomFactor - ZOOM_BASE;
                double fitScaling = page->GetFitScaling(m_screenWidth, m_screenHeight, g_resolutionX, g_resolutionY);
                double factor = 1.0;
                if ( delta >= 0.0 ){
                    factor = log(m_zoomFactor);
                } else {
                    factor = 1.0 - (1.0 / ( 1.0 + exp(delta)) - 0.5) * 2;
                }
                scaling = fitScaling * factor;

                //LOG(DEBUG) << "delta: " << delta << " factor: " << factor << " scaling=" << scaling << " offset=(" << pixelX << "," << pixelY << ")";

                m_cairoRender->SaveState();
                //if ( pixelX != m_origPixelX || pixelY != m_origPixelY || scaling != m_origScaling ){
                    ofd::Render::DrawParams drawParams = std::make_tuple(pixelX, pixelY, scaling);
                    m_cairoRender->DrawPage(page, drawParams);
                    m_origPixelX = pixelY;
                    m_origPixelY = pixelY;
                    m_origScaling = scaling;
                //}
                m_cairoRender->Paint(surface);
                m_cairoRender->RestoreState();
            } else {
                LOG(ERROR) << "page->Open() failed. pageIndex=" << m_pageIndex;
            }
        }
    }
}

// -------- loadTexture() --------
SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &image_file){
    SDL_Texture *texture = nullptr;

    SDL_Surface *loadedSurface = IMG_Load(image_file.c_str());
    if ( loadedSurface != nullptr ){
        texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        if ( texture == nullptr ){
            LOG(WARNING) << "SDL_CreateTextureFromSurface() failed. " << SDL_GetError();
        }
        SDL_FreeSurface(loadedSurface);
    } else {
        LOG(WARNING) << "IMG_Load(\"" << image_file << "\") failed " << SDL_GetError();
    }

    return texture;
}

#include <gflags/gflags.h>
DEFINE_int32(v, 0, "Logger level.");
DEFINE_string(owner_password, "", "The owner password of PDF file.");
DEFINE_string(user_password, "", "The user password of PDF file.");
int main(int argc, char *argv[]){

    TIMED_FUNC(timerMain);

    gflags::SetVersionString("1.0.0");
    gflags::SetUsageMessage("Usage: ofdviewer <pdffile>");
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    Logger::Initialize(FLAGS_v);

    LOG(INFO) << "Start " << argv[0];

    std::string filename = argv[1];

    ofd::PackagePtr package = std::make_shared<ofd::Package>();
    if ( !package->Open(filename) ){
        LOG(ERROR) << "OFDPackage::Open() failed. filename:" << filename;
        return -1;
    }
    DocumentPtr document = package->GetDefaultDocument(); 
    assert(document != nullptr);
    LOG(DEBUG) << document->to_string();

    bool bOpened = document->Open();
    if ( !bOpened ){
        LOG(ERROR) << "Open OFD Document failed. filename: " << filename;
        exit(-1);
    }

    size_t total_pages = document->GetNumPages();
    LOG(INFO) << total_pages << " pages in " << filename;
    if ( total_pages > 0 ){
        int screenWidth = 794;
        int screenHeight = 1122;
        int screenBPP = 32;

        MySDLApp app("OFDViewer", screenWidth, screenHeight, screenBPP, document);
        
        app.Execute();
    }

    package->Close();



    LOG(INFO) << "Done.";

    return 0;
}

