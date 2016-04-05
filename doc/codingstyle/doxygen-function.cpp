class Debugger {
    public:
        // ...

        /**
         * @brief Write frame capture into a file
         * @param filename      Filename to write the output into. Format is
         *      autodetected from the extension, supported formats are  `*.png`
         *      and `*.jpg`.
         * @param format        Pixel format
         * @param dataLayoutFlags Data layout flags
         * @return File size on success, `0` on failure
         *
         * Writes a frame capture with currently enabled debug watermark into a
         * file.
         *
         * If the file in given location already exists, the filesystem is not
         * writable or the filesystem is full, returns `0`. The @p dataLayoutFlags
         * and @p format are expected to be compatible with current framebuffer
         * setup, otherwise an assertion is fired.
         */
        std::size_t writeFrameCapture(const std::string& filename, PixelFormat format, DataLayoutFlags dataLayoutFlags);
};
