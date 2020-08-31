
loadMap(){
    map = new Tmx::Map();
    map->ParseFile(mapFileName);
    if (map->HasError())
    {
        MLOG_INFO << "load tmx map error code: " << map->GetErrorCode();
        MLOG_INFO << "tmx map error text: "<< map->GetErrorText().c_str();
    }
    printf("Version: %1.1f\n", map->GetVersion());
    printf("Orientation: %d\n", map->GetOrientation());
    if (!map->GetBackgroundColor().empty())
        printf("Background Color (hex): %s\n",
                map->GetBackgroundColor().c_str());
    printf("Render Order: %d\n", map->GetRenderOrder());
    if (map->GetStaggerAxis())
        printf("Stagger Axis: %d\n", map->GetStaggerAxis());
    if (map->GetStaggerIndex())
        printf("Stagger Index: %d\n", map->GetStaggerIndex());
    printf("Width: %d\n", map->GetWidth());
    printf("Height: %d\n", map->GetHeight());
    printf("Tile Width: %d\n", map->GetTileWidth());
    printf("Tile Height: %d\n", map->GetTileHeight());

    for (int i = 0; i < map->GetNumTilesets(); ++i)
    {
        printf("                                    \n");
        printf("====================================\n");
        printf("Tileset : %02d\n", i);
        printf("====================================\n");

        // Get a tileset.
        const Tmx::Tileset *tileset = map->GetTileset(i);

        // Print tileset information.
        printf("Name: %s\n", tileset->GetName().c_str());
        printf("Margin: %d\n", tileset->GetMargin());
        printf("Spacing: %d\n", tileset->GetSpacing());
        printf("First gid: %d\n", tileset->GetFirstGid());
        printf("Image Width: %d\n", tileset->GetImage()->GetWidth());
        printf("Image Height: %d\n", tileset->GetImage()->GetHeight());
        printf("Image Source: %s\n", tileset->GetImage()->GetSource().c_str());
    }
}
