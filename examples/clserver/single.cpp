class ssdbSingle
{
    private:
        // Private Constructor
        ssdbSingle();
        // Stop the compiler generating methods of copy the object
        ssdbSingle(ssdbSingle const& copy);            // Not Implemented
        ssdbSingle& operator=(ssdbSingle const& copy); // Not Implemented

    public:
        static ssdbSingle& getInstance()
        {
            // The only instance
            // Guaranteed to be lazy initialized
            // Guaranteed that it will be destroyed correctly
            static ssdbSingle instance;
            return instance;
        }
};
