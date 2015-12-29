// ----------------------------------------------------------------------------
// situbeRender.h: load geometry in the format of "tgdata" and then wrap each
//					each streamline with rings resembling tubes. 
//
//					Note that the tubes produced are not of any anatomical
//					meanings but simply a geometrical artifacts. So they are not
//					genuine streamtubes
//
// Creation : Feb. 6th 2011
//
// Revision:
//	@Feb. 21th 
//		1.Geometry-based direction color encoding
//		2.add interaction option for using vertex coordinate as vertex normal 
//	@Feb. 24th
//		1. correct the problem of negative colors in the orientation color
//		encoding. By take fabs over the negative ones, this coloring becomes
//		a pure orientation color encoding scheme
//	@Feb. 26th
//		1. add a continuous and anatomically symmetric coloring encoding by
//			scaling the gamut of [-1,1] to [0,2] and then to [0,1]
//		2. add multiple coloring scheme support for coloring comparative
//		analysis
//	@Feb. 27th
//		1. extract coloring scheme stuff into an independent component as a
//			template class CColorMapper in colorscheme.h
//	@Feb. 28th
//		1. add an anatomical coordinate system, together with the spheric color
//		map gadget
//	@Mar. 2nd
//		1. add the DWI image embedding, held by one more gadget : CDcmExplorer
//		2. in order to ease implementation of mitubeRender, this single model
//		render, originally held in a single cpp, is mutated into an ordinary
//		class design and then instantiated in a boosting main-like cpp. Here
//		thus comes this header file
//	@Mar. 4th
//		1.add glutIdleFunc callback to support an IPC mechanism to receive user
//		interaction from other sinlge DTI model rendering process so that to
//		implement a synchronous interaction response, as is expressly serving
//		the MitubeRender
//	@Mar. 5th
//		1.add boolean flag to adapt the CSitubeRender class for building both a
//		single render and serving the CMitubeRender for building multiple-window
//		DTI model render - simply using m_nSiblings==0 as the boolean
//		2. add SIGUSR1 handler to receive the notification from parent that one
//		more sibling process died and thus to cut off the number of alive
//		siblings by 1, this is non-trivial since it will help CSitubeRender keep
//		aware of the latest number of siblings thus make possible the
//		interaction synchronization will still be correct even some of the
//		processes terminate on the way
//	@Mar. 6th
//		1.Fix the bug that when button is bounced up, the IPC mechanism does not
//		count when the event dispatching has finished, causing the event to be
//		kept being dispatched thus raising disorders, by forcing
//		glutPostRedisplay() even in the responser to button event while the
//		state being GLUT_UP, both in GLcppmoth and iselbox classes
//		2.When receiving SIGTERM, this process, mostly in the role of child
//		process, should grasp the last chance to release all resources,i.e to
//		invoke cleanup
//		3.Change special key response: END to terminate current process,
//		PAGE_DOWN to terminate all members of the process group
//		4.To enhance the flexibility in terms of being managed as
//		children/component, add a m_bSuspended switch to allow the manager to
//		postpone the entrance into GL event loop
//	@Mar. 9th
//		.add interaction help prompt gadget, a GL textbox; and an extra command
//		line argument for loading help text from a file; another key mapping is
//		also added accordingly as a switch of help text box presence:
//			F1 - switch help text box presence
//		.add task prompt gadget, a subtyped CGLTextbox; and an extra command
//		line argument for loading the task list file
//	@Mar. 10th
//		.for performing visualization tasks, going to the next task as the response
//		 to Space bar pressing is added
//		.Before each task is confirmed to start, nothing will be drawn on the
//		screen except the task box and all other inputs are blocked as well then
//		except for the Space bar
//	@Mar. 11th
//		.Two types of associative relation among all selection boxes are
//		defined: 
//			AND pattern - conditions must be met for all boxes (for selection
//			mode, a fiber is selected only if it goes through all boxes; for
//			removal mode, a fiber is removed only if it is outside of all
//			boxes);
//			OR pattern = conditions need be met for at least one boxes(for
//			selection mode, a fiber is selected if only one of the boxes covers
//			it; for removal mode, a fiber is culled if only one of the boxes
//			excludes it)
//		and another key shortcut is added accordingly
//			F2 - switching associative pattern among selection boxes
//		.Logic in isTubeInSelbox() is changed to be consistent with isLineInBox,
//		which is actually required to make sure that the selection/removal mode,
//		either in AND or OR pattern, works correctly geometrically.
//	@Mar. 12th
//		1.For the visualization study's purpose, saving/loading partial brain
//		model, one or certain other number of regions, say, is added as an
//		auxiliary functionality; this mainly impact CTgdataLoader::dump though,
//		and the change needed in CSitubeRender is the addition of new member
//		function dumpRegions(), as might be the response to key stroke or part
//		of process upon a task is done
//		--- the m_edgeflags member in parent class CGLIBoxApp is put into use
//		for this purpose, which is in fact meant for implementing vertex
//		selection that is not in use in CSitubeRender where we do not use vertex
//		flag to determine a streamline is in a box or not. So it is safe to make
//		this very reuse of the seemingly deprecated member here in CSitubeRender
//		--- F11 is mapped for this functionality.
// @Mar. 15th
//		1.Still for visualization study's purpose, saving a single or a set of
//		fiber bundles by dumping the indices of fibers currently selected by
//		box(es). The first line of the resulting text file will be the number of
//		fibers dumped with their indices.
//		--- m_edgeflags is used again for the implementation.
//		--- F12 is mapped for this functionality.
// @Mar. 16th
//		1. for some tasks in visualization study, it is solicited to report the
//		box information, two corners particularly, at appropriate time; this can
//		be useful if we want to fathom a space inside the DTI model where I will
//		place an extraneous object, since I can locate the bounding box of the
//		would-be outsider by using the box
//		--- PAGE_UP is mapper for reporting box itself anytime; (Oops, we have
//		used up all possible function keys F1~F12...)
//		NOTE: since for the purpose of location probing we need at most one box,
//		so this function is just to call m_boxes[0].reportself()
// @Mar. 17th
//		1. for consistency, box information is also dumped into a text file,
//		since it will be used later as is loaded from a file.
//		2. sub-function is formed by extracting the verisimilar logic for
//		randomly constructing the name of a file under the same directory as
//		another input file and made discernible by suffixing the time in seconds
//		3. key mapping for exiting along with all siblings if any is changed
//		from PAGE_DOWN to INSERT. and PAGE_DOWN is noe mapped for dumping the
//		box information while telling the box is then separated from fibers
//		and PAGE_UP, mapped for dumping box information as well, is to tell the
//		box is then tangential to fibers around. (we originally desired to use
//		DELETE instead of INSERT since the former is more intuitive, it is
//		pityful however DELETE is not defined as a function key in standard
//		GLUT, it is actually defined in extended GLUT though.
// @Mar. 25th
//		.Trivial improvement: when orientation coloring is disenable, the color
//		map gadget should reflect the updated status, which is done simply by
//		setting the color schme to CLSCHM_CUSTOM, or more brutely, disable the
//		color map gadget, at that time
// @April. 12th
//		.Precompilation macros added to control the integration DWI Image 
//		Embedding selectively
// @April. 18th
//		.m_bRemovalbased and m_bOR are moved upwards to CGLIBoxApp since this
//		parent class charges all box-related interactions
//		.help text box is moved further upwards to CGLApp since it is really a
//		common feature neaded for general GL applications
// @April. 25th
//		.Add streamline model rendering, mixed with the original streamtube
//		model, to provide a context for selecting ROIs; user can toggle the
//		context like using other binary controls
// @May 9th
//		.Add skeleton projection as the guide to the 3D exploration; the
//		skeleton is produced as the silhouette of fiber bundles clusterd by the
//		distance similarity measurement (result of Cad's clustering script).
//		------- the skeleton is represented by a set of streamlines all stored
//		in an auxiliary .tgdata.
//		.A rectangular canvas is added to provide a background to help discern
//		the skeletonic streamlines
// @May 16th
//		.Encase each of the procedures for auxiliary rendering in
//		CSitubeRender::draw() into a function to make the code clearer so as to 
//		facilitate maintenance and extension
//		.Add grids on the ground canvas
// @May 17th
//		.The long probing problem: skeleton projection with the same  intuitive
//		transformation as that of the main geometries, gets finally corrected!
// @May 19th
//		.Fix the bug that projectSkeleton(0,0) might be executed repeatedly...
//		by an extra variable to record the status on initialization of the
//		projection
// @May 30th 2011
//		.implement box-owned independent selection mode control
//		.isLineInBox has been strengthened in the parent class CGLIBoxApp thus
//		tube culling implementation is unified to using isLineInBox
//		consistently, and left isTubeInSelbox useless
// @June 1st 2011
//		.Enable the idle function for self exhibition by rotating around
//		different axes
// @June 4th 2011
//		.incorporate ROI highlighting (by yellowing fibers in the ROI) feature
//		designed for certain tasks of past study
//		.incorporate preliminary selection boxes loading feature, also designed
//		for certain tasks of past study
// @June 6th 2011
//		.Provide streamtube context: using current streamtubes when beining
//		rendered in translucency as context, overlay another copy but with
//		monochromatic yellow color as the main fibers to select from
// @June 7th 2011
//		.Overhaul box filtering logic for new requirements:
//			a. the streamtube context should also be enabled to be filtered by
//			boxes, but only by Removal boxes;
//			b. both the Selection and Removal boxes can work on the primary
//			fibers
//		.Fix a bug: switching brushing mode back to non-brushing should not
//		resume fibers that has been brushed out
//		. To dodge the abomination of medical experts like Arkus, set the first
//		box added a quite smaller size, instead of the original bounding size
//		. Add halo effect onto the contextual streamtubes to enhance the depth
//		perception	
// @June 8th 2011
//		.Optimize the performance ---- Detecting collisions only when this is
//		any box changes
// @June 10th 2011
//		.add extra control for 
//			- tube radius, increase by 'r' and decrease by 'R'
//			- tube LOD, increase by 'x' and decrease by 'X'
//			- tube alpha, increase by 'a' and decrease by 'A'
//			- Halo effect switching by 'O'
//		.correct the skeleton filtering by boxes, now the skeleton only follows
//		the streamtube context
//		.since separate flag updating, while benefitting the global
//		transformation, can impair the performance of box operation (because box
//		operation causes fiber cutting), add extra boolean switch to make
//		available a choice between the separate update and update in draw()
// @June 14th 2011
//		.For extension to CSitubeRenderMult, all private members are upgraded to
//		protected to offer convenience for descendants
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#ifndef  _SITUBERENDER_H_

#ifdef DWI_EMBEDDING
#include "DCMviewer.h"
#endif
#include "colorscheme.h"
#include "GLgadget.h"
#include "GLoader.h"
#include "GLiboxmoth.h"
#include "glrand.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/signal.h>

using std::cout;
using std::cerr;
using std::vector;
using std::string;
using std::set;
using std::ifstream;

// possible user input event
typedef enum _input_event_t {
	IE_KEY_PRESSED = 0x00,
	IE_SPECIAL_KEY,
	IE_MOUSE_CLICKED,
	IE_MOUSE_MOVE,
	IE_MOUSE_PASSIVE_MOVE,
	IE_MOUSE_WHEEL_ROLL,
	IE_NONE = -1
}input_event_t;

class CSitubeRender: public CGLIBoxApp {
protected:
	/*
	 * this is actually delineating a piece of shared memory holding information
	 * on user interaction which is meant for communciating between multiple
	 * processes of DTI model render
	 */
	typedef struct _interaction_info_t {
		// flagging current key or mouse event, an enumeration of input_event_t
		int event; 
		// mouse position while either clicking or moving
		int x,y;
		// mouse button and the state
		int button, state;
		// key pressed, wheel pushed and the wheel rolling direction
		int key, wheel, direction;

		// indicate if new interaction input reached
		bool bUpdated;

		// current number of processes already synchronized
		int nFinished;
	}interaction_info_t;

	static const char* SITR_SHM_NAME;

public:
	CSitubeRender(int argc, char **argv);
	virtual ~CSitubeRender();

	// randomly offering a color for the uniform coloring
	void genColors();

	// decide the longest and shortest streamline segment among all streamtubes
	// in order to determine tube radius automatically
	void findMaxMinSegLen();

	/*
	 * MAKING artificial tubes by wrapping rings around a single streamline
	 * given by the index of the streamline storage, as is a linear structure
	 * created by loading the input geometry of streamlines
	 */
	void buildTubefromLine(unsigned long lineIdx);

	/*
	 * serialize the streamtube geometry generated into a file in the format
	 * of WaveFront obj
	 */
	int serializeTubes(const string& fnobj);

	/* 
	 * read a file in the format of "tgdata", a .data ouput from tubegen into a
	 * series of linear array as vectors, each holding a streamline including
	 * its vertices and colors, then all these vectors are indexed in a headlist
	 * in which elements are pointers to these vectors respectively
	 *
	 * After the streamline geometry loaded, streamtube geometry was calculated
	 * by wrapping around each streamline a slew of rings that is expected to
	 * simulate a shape of tube only in the geometrical sense, not pertinent to
	 * visually encoding tensor attributes like the eigenvectors and eigenvalue
	 * in each voxel in the tensor field
	 */
	int loadGeometry();

	/*
	 * judge if the tube indicated by the index within the streamtube store is
	 * inside all selection boxes
	 */
	//bool isTubeInSelbox(unsigned long lineIdx);
	bool isTubeInSelbox(const vector<GLfloat>& line);

	// overloadings to those in the parent class for customized actions
	void glInit(void);
	int handleOptions(int optv);

	void keyResponse(unsigned char key, int x, int y);
	void specialResponse(int key, int x, int y);
	void mouseResponse(int button, int state, int x, int y);
	void mouseMotionResponse(int x, int y);
	void mousePassiveMotionResponse(int x, int y);
	void mouseWheelRollResponse(int wheel, int direction, int x, int y);

	int mainstay();
	void draw();
	void onIdle(void);
	void onReshape( GLsizei w, GLsizei h );

	// tell the number of sibling processes in order to realize an imperative
	// interaction input synchronization
	void setNumberOfSiblings(int n);

	// update the suspense switch
	void suspend(bool bSuspend = false);

	// query the sync status
	bool isSync() const;

	// dump the geometry of streamlines associated with streamtubes currently
	// visible( i.e. selected ) into a file of the given name in the format of
	// "tgdata" as exactly the same as that of the file where the streamline
	// geometry is loaded;
	// when the file name is empty string, a fabricated name will be used - the
	// value of -f option plus current time in seconds (this can make unique the
	// naming)
	int dumpRegions(const char* fnRegion="");

	// dump indices of fibers currently selected into a text file, where each
	// line will store a single fiber in terms of its index in the streamtube
	// store m_alltubevertices
	int dumpFiberIdx(const char* fnFiberidx="", const char* firstline="");

	// dump current box location information into a text file
	int dumpBoxpos(int bidx, const char* fnBoxpos="", const char* firstline="");

protected:
	/* when coloring schemes are all not used, this color is the uniform color
	 * for all tubes which is customizable in the run-time*/
	GLfloat m_colors[3];
	/* a trivial switch telling if to use the color originally produced during
	 * fiber tracking for each streamline */
	GLboolean m_bUseOrgColor;
	/* a binary switch too telling if to apply direction/orientation color
	 * encoding */
	GLboolean m_bUseDirectionColor;
	/* used to create a special visual effect, taking vertex coordinate as the
	 * normal for the cooresponding vertex */
	GLboolean m_bVnormal;

	/* the contained sub-object used for source geometry loading and parsing */
	CTgdataLoader m_loader;

	/* used for loading the skeleton geometry */
	CTgdataLoader m_skeletonLoader;
	CTgdataLoader m_orgSkeleton, m_staticSkeleton;

	/* edge flags for skeleton fibers */
	vector<GLboolean>	m_skeletonEdgeflags;
	vector<GLboolean>	m_brushedoutSkeletonflags;

	/* source file holding the most essential streamline geometry */
	string m_strfnsrc;
	/* file to store dumped geometry in the format of OBJ */
	string m_strfnobj;
	/* directory holding DWI images to embed when needed, mostly b0 images
	 */
	string m_strdwidir;
	/* text file holding a list of tasks for a single session */
	string m_strfntask;

	/* another streamline geometry as the fiber bundle skeleton associated with
	 * the primary geometries in m_strfnsrc
	 */
	string m_strfnskeleton;

	/* Level of Detail, the granularity of interpolation in the fabrication of
	 * streamtube geometry*/
	GLubyte				m_lod;
	/* number of selection box */
	GLubyte				m_nselbox;

	/* if use variant radius for tubes */
	GLboolean			m_bVradius;

	/* fantastic factor tunning the streamtube generation */
	GLfloat m_fAdd;

	/* tube radius and radius of the bounding box of the streamline model */
	GLfloat m_fRadius, m_fbdRadius;

	/* the maximal and minimal length over all streamline segments */
	GLfloat m_fMaxSegLen, m_fMinSegLen;

	/* stash of streamtube geometry for all streamlines  */
	vector< vector<GLfloat> >		m_alltubevertices;
	vector< vector<GLfloat>	>		m_alltubenormals;
	vector< vector<GLfloat>	>		m_alltubecolors;
	vector< vector<GLfloat>	>		m_encodedcolors;
	vector< vector<GLuint> >		m_alltubefaceIdxs;

	/* context streamtubes */
	vector< vector<GLfloat> >		m_ctxtubevertices;
	vector< vector<GLfloat>	>		m_ctxtubecolors;

	/* get the maximal X,Y and Z coordinate among all vertices */
	GLdouble m_maxCoord[3];	
	/* get the minimal X,Y and Z coordinate among all vertices */
	GLdouble m_minCoord[3];	

	/* color mapper responsible for color encoding */
	CColorMapper<GLfloat> m_colormapper;
	/* currently we might want to compare multiple coloring encoding scheme */
	int m_colorschemeIdx;
	/* a spherical color map gadget */
	CGLGadget* m_pcmGadget;
	/* anatomical axes gadget */
	CGLGadget* m_paxesGagdet;

#ifdef DWI_EMBEDDING
	/* A DICOM DWI IMAGE explorer */
	CDcmExplorer m_dcmexplorer;

	/* DWI Image embedding switch */
	bool m_bShowDWIImage;
#endif

	/* a task prompt gadget */
	CGLTaskbox m_taskbox;

	/* point to the shared memory structure of interaction information */
	interaction_info_t* m_pIntInfo;

	/* switching synchronous mode */
	bool m_bSync;
	/* in order to force interaction input synchronization, we need to know the
	 * number of sibligns
	 */
	int m_nSiblings;

	// to bypass the callback function limitations and OO design
	// : we want to visit non-static members from a static member (seems
	// evil...)
	static CSitubeRender* m_psitInstance;

	/* to suspend entrance into GL event loop or to release it to make enter */
	bool m_bSuspended;

	/* if show context - the streamline mode */
	bool m_bShowContext;

	/* if the skeleton projection has been initializd */
	bool m_bSkeletonPrjInitialized;

	/* if adopting the brush style, i.e. fiber removed will not automatically
	 * resume after the box moved away
	 */
	bool m_bBrushEffect;

	// file containing indices of expectedly selected fibers
	std::string m_strfnFiberIdx;

	// set of the loaded fiber indices
	std::set<unsigned long> m_expectedFiberIndices;

	std::vector< std::string > m_strfnTumorBoxes;
	// the opposite corner of the tumor bounding box
	std::vector< point_t > m_tumorBoxMin, m_tumorBoxMax;

	/* we enforce the user must press all these keys at least once
	 ********************************
					
				        PgDn F12 F11
					      |   |   | 
	---------------------------------
	| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
	---------------------------------
	*/
	unsigned char m_pressBin;

	// context vertex flags
	vector<GLboolean>	m_ctxedgeflags;

	// for this control: those fibers brushed out, i.e. culled by brushing
	// boxes, should not be automatically resumed when the brushing mode
	// gets revoked; at the same time, the old Removal mode should still be
	// working correctly as it did when this mode is set : fibers in these boxes
	// are only withhold when they are currently hit by boxes, and will
	// automatically resume when boxes move away - release those fibers
	vector<GLboolean>	m_brushedoutflags;

	// halo effect switch
	bool	m_bHaloEffect;
	
	// streamtube context transparency
	GLfloat	m_fAlpha;

	// if using separate flag updating
	bool	m_bSeparateFlagUpdate;

protected:
	// extend the box button function
	void handleBtnEvents(int event);

	// draw streamline model as context
	void drawContext();

	// draw streamtube model as context
	void drawTubeContext();

	// draw haloes, which is in fact the outlines of streamtubes
	void drawHaloes();

	// project fiber bundle skeletons to the XZ plane
	int projectSkeleton(int x, int y, bool bZ = false);
	// draw skeleton fibers
	void drawSkeleton();

	/*
	 * @brief open (or create for this process launched) the shared memory and
	 * initialize it
	 * @param bInit a boolean telling if to initialize after opening/creating
	 * @return 0 for success negative values, -1, say, for failure
	 */
	int	_initSharedInfo(bool bInit=true);

	/*
	 * @brief release the shared memory reference
	 * @return 0 for success negative values, -1, say, for failure
	 */
	int	_releaseSharedInfo();

	/*
	 * @brief this is the real stuff to be invoked as the response to key
	 * pressing
	 */
	void _realkeyResponse(unsigned char key, int x, int y);

	/*
	 * @brief this is the real stuff to be invoked as the response to special
	 * key pressing
	 */
	void _realSpecialResponse(int key, int x, int y);

	/*
	 * @brief treat the signal of SIGUSR1 and then update the number of alive
	 * siblings
	 */
	static void _on_sibling_exit(int sig);

	/*
	 * @brief treat the signal of SIGTERM and then release all resources that
	 * would not be automatically released when the process is forced to
	 * terminate
	 */
	static void _on_killed(int sig);

	/*
	 * @brief form a name for a file under the same directory as that of the
	 *  given another file
	 * @param fnout the resulting file name with full path as fnsrc
	 * @param prefix a string giving the file name prefix
	 * @param fnsrc a string giving the reference file full path from where the
	 * directory will be extracted.
	 * @param usetime a boolean giving if suffixing with current time in seconds
	 * @param extension a string giving a different extension from the given
	 * file. by default the extension will be that of the given file
	 * @return 0 for success and any other value for failure
	 */
	static int _formFilename(std::string& fnout, const std::string& prefix,
			const std::string& fnsrc, bool usetime = true, const char* extension="");

	// load fiber indices from a file where the indices of fibers constituting a
	// predefined fiber bundle are prepared therein.
	int _loadFiberIdx();

	// load bounding box information for the tumor potato
	int _loadTumorBoxpos();

	// update all possibly required vertex flags
	void _updateAssortedFlags();
};

#endif // _SITUBERENDER_H_

/* set ts=4 sts=4 tw=80 sw=4 */

