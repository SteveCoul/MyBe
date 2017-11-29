
#include "Options.hpp"

#include "xlog.hpp"

int Options::parse( int argc, char** argv ) {
	int rc = 0;
	char error_text[256];
	char* app = argv[0];

	argc--;
	argv++;

	error_text[0] = 0;

	while ( argc != 0 ) {
		bool skip = false;
		if ( argv[0][0] != '-' ) {
			if ( argc == 1 ) {
				m_dest_filename = argv[0];
			} else {
				strcpy( error_text, "output file must be specified on end" );
				rc = -1;
				break;
			}
		} else if ( argv[0][1] == '-' ) {
			sprintf( error_text, "%s not recognized", argv[0] );
			rc = -1;
			break;
		} else if ( argv[0][2] != '\0' ) {
			sprintf( error_text, "%s not recognized", argv[0] );
			rc = -1;
			break;
		} else {
			switch( argv[0][1] ) {
			case 'h':
				rc = -1;
				break;
			case 's':
				m_save_videos = true;	
				break;
			case 'd':
				m_decode_videos = true;
				break;
			case 'i':
				m_source_filename = argv[1];
				skip = true;
				break;
			default:
				sprintf( error_text, "%s invalid", argv[0] );
				rc = -1;
				break;
			}
		}		
		if ( skip ) {
			argv++;
			argc--;
		}
		argv++;
		argc--;
	}
	if ( rc != 0 ) {
		fprintf( stderr, "\n%s <options> [output_name]\n", app );
		fprintf( stderr, "\n" );
		fprintf( stderr, "    Default output name : %s\n", m_dest_filename.c_str() );
		fprintf( stderr, "    -h              Help\n" );
		fprintf( stderr, "    -i <name>       Set input name [default: %s]\n", m_source_filename.c_str() );
		fprintf( stderr, "    -s              Save intermediate video streams\n" );
		fprintf( stderr, "    -d              Decode intermediate video streams\n" );
		fprintf( stderr, "\n" );
		if ( error_text[0] != '\0' ) fprintf( stderr, "Error: %s\n", error_text );
	}
	return rc;
}

void Options::erase() {
}

void Options::setDefault() {
	m_source_filename = "input.ts";
	m_dest_filename = "output.ts";
	m_save_videos = false;
	m_decode_videos = false;
}

void Options::dump() {
	XLOG_INFO( "Parameters:" );
	XLOG_INFO( "source filename %s", m_source_filename.c_str() );
	XLOG_INFO( "destination filename %s", m_dest_filename.c_str() );
	XLOG_INFO( "Save Videos? %s", m_save_videos ? "Y" : "n" );
	XLOG_INFO( "Decode Videos? %s", m_decode_videos ? "Y" : "n" );
}

