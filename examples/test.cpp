#include "contrib/minizip/zip.h"
#include "contrib/minizip/iowin32.h"

int main(int argc, char** argv)
{
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64W(&ffunc);
    wchar_t fl[] = L"abc.zip";
    zipFile zf = zipOpen2_64(fl, APPEND_STATUS_CREATE, NULL, &ffunc);
    zipOpenNewFileInZip64(zf, "test.txt", NULL, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION, 0);
    zipWriteInFileInZip(zf, "test", 4);
    zipCloseFileInZip(zf);
    zipClose(zf, NULL);
}
