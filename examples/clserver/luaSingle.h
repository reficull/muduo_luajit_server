/*
class Lua_State
{
    lua_State *L;
    public:
    Lua_State() : L(luaL_newstate()) {
        // load Lua libraries
        luaL_openlibs(L);
    }

    ~Lua_State() {
        lua_close(L);
    }

    // implicitly act as a lua_State pointer
    inline operator lua_State*() {
        return L;
    }
};
*/
class luaSingle
{
    private:
        // Private Constructor
        luaSingle():L(luaL_newstate()){
            luaL_openlibs(L);
        }
        // Stop the compiler generating methods of copy the object
        luaSingle(luaSingle const& copy);            // Not Implemented
        luaSingle& operator=(luaSingle const& copy); // Not Implemented

    public:
        lua_State *L;
        static luaSingle& getInstance()
        {
            // The only instance
            // Guaranteed to be lazy initialized
            // Guaranteed that it will be destroyed correctly
            static luaSingle instance;
            return instance;
        }
};
 
        int hset(const std::string &name, const std::string &key, const std::string &val)
        {
            ssdb::Status s;
            s = client->set("k", "hello ssdb!");
            if(s.ok()){
                printf("k = hello ssdb!\n");
            }else{
                printf("error!\n");
            }
            return 0;
        }
