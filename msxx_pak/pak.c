const char* get_pakFile_category(const unsigned int subDir)
{
	switch (subDir) {
	case 0x01: {
		return "cmp: tile position data";
		break;
	}
	case 0x02: {
		return "anm: tile animation data (?)";
		break;
	}
	case 0x03: {
		return "frm: unknown";
		break;
	}
	case 0x04: {
		return "chr: tile data";
		break;
	}
	case 0x05: {
		return "obj_pal";
		break;
	}
	case 0x06: {
		return "Mission 4(?) unknown data";
		break;
	}
	case 0x07: {
		return "Mission 4(?) screen rolling(?) (unknown)";
		break;
	}
	case 0x08: {
		return "GUI fonts";
		break;
	}
	case 0x09: {
		return "acx SE bank (contain adx)";
		break;
	}
	case 0x0a: {
		return "acx voice bank (contain adx)";
		break;
	}
	case 0x0b: {
		return "Stage Attribute (Terrain data)";
		break;
	}
	case 0x0c: {
		return "eset (Object layout)";
		break;
	}
	case 0x0d: {
		return "es2p (Object layout 2p(?))";
		break;
	}
	case 0x0e: {
		return "eset_sur (Object layout too(?))";
		break;
	}
	case 0x0f: {
		return "map \"inf\" data: background loading distance(?)";

		break;
	}
	case 0x10: {
		return "map \"tbl\" data: unknown";
		break;
	}
	default:
		return "unknown";
		break;
	}
}
