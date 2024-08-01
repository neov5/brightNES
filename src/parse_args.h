// SPDX-License-Identifier: Apache-2.0
// Copyright 2019 Erik Nyquist

/*
 * Erik Nyquist <eknyquist@gmail.com>
 * Command line argument parser
 *
 * API definition
 */

#include <stdlib.h>
#include <string.h>

#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H


/**
 * Defines a named option that requires an additional argument
 *
 * @param  short_flagstr   String to expect for the short version of the option
 *                         flag. Should begin with at least one "-" character,
 *                         e.g. "-f"
 * @param  long_flagstr    String to expect for the long version of the option
 *                         flag. Should begin with at least one "-" character,
 *                         e.g. "--file". This value can be set to NULL if no
 *                         long version is needed.
 * @param  argtype         Argument datatype, must be of type argtype_e
 * @param  argdata         Pointer to an object of type #argtype to copy the
 *                         decoded argument data
 */
#define ARGS_OPTION(short_flagstr, long_flagstr, argtype, argdata) { \
    .short_flag = short_flagstr, \
    .long_flag = long_flagstr, \
    .arg_type = argtype, \
    .opt_type = OPTTYPE_OPTION, \
    .data = argdata, \
    .seen = 0 \
}


/**
 * Defines a positional argument
 *
 * @param  argtype         Argument datatype, must be of type argtype_e
 * @param  argdata         Pointer to an object of type #argtype to copy the
 *                         decoded argument data
 */
#define ARGS_POSITIONAL_ARG(argtype, argdata) { \
    .short_flag = NULL, \
    .long_flag = NULL, \
    .arg_type = argtype, \
    .opt_type = OPTTYPE_POSITIONAL, \
    .data = argdata, \
    .seen = 0 \
}


/**
 * Defines an named option that does not require an additional argument. This
 * type of option behaves as a boolean input-- if the flag is present in the
 * argument list, the value is set to 1. Otherwise, the value is set to 0.
 *
 * @param  short_flagstr   String to expect for the short version of the option
 *                         flag. Should begin with at least one "-" character,
 *                         e.g. "-f"
 * @param  long_flagstr    String to expect for the long version of the option
 *                         flag. Should begin with at least one "-" character,
 *                         e.g. "--file". This value can be set to NULL if no
 *                         long version is needed.
 * @param  value           Pointer to an int, to write the flag status to. A
 *                         value of 1 means the flag is present, 0 means the
 *                         flag is not present.
 */
#define ARGS_FLAG(short_flagstr, long_flagstr, value) { \
    .short_flag = short_flagstr, \
    .long_flag = long_flagstr, \
    .arg_type = ARGTYPE_NONE, \
    .opt_type = OPTTYPE_FLAG, \
    .data = value, \
    .seen = 0 \
}


/**
 * A list of option definitions must end with this sentinel value
 */
#define ARGS_END_OF_OPTIONS { \
    .short_flag = NULL, \
    .long_flag = NULL, \
    .arg_type = ARGTYPE_NONE, \
    .opt_type = OPTTYPE_NONE, \
    .data = NULL, \
    .seen = 0 \
}


/**
 * Available data types for arguments
 */
typedef enum {
    ARGTYPE_INT = 0,
    ARGTYPE_LONG,
    ARGTYPE_UINT,
    ARGTYPE_ULONG,
    ARGTYPE_FLOAT,
    ARGTYPE_DOUBLE,
    ARGTYPE_STRING,
    ARGTYPE_HEX,
    ARGTYPE_NONE
} argtype_e;


/**
 * Available option types
 */
typedef enum {
    OPTTYPE_FLAG,
    OPTTYPE_OPTION,
    OPTTYPE_POSITIONAL,
    OPTTYPE_NONE
} opttype_e;


/**
 * Structure to hold data required for a single option
 */
typedef struct {
    int seen;
    const char *short_flag;
    const char *long_flag;
    argtype_e arg_type;
    opttype_e opt_type;
    void *data;
} args_option_t;


/**
 * Parse all commands line arguments
 *
 * @param  argc     Argument count, passed directly from main()
 * @param  argv     Argument list, passed directly from main()
 * @param  options  List of option definitions
 *
 * @return          0 if argument parsing succeeded, -1 if an error occurred
 */
int parse_arguments(int argc, char *argv[], args_option_t *options);


/**
 * Print an error message (using printf) describing the error that caused
 * parse_args to fail. If no error has occurred, then nothing will be printed.
 */
void parse_arguments_print_error(void);


/**
 * Return a pointer to an string describing the error that caused parse_args to
 * fail. If no error has occurred, NULL is returned.
 *
 * @return   Pointer to a string describing the error, or NULL for no error
 */
char *parse_arguments_error_string(void);


/*
 * End of the API definition. The rest of this file contains the implementation.
 */


#define ARRAY_LEN(array) (sizeof(array) / sizeof((array)[0]))

// Max. length of an option string
#define MAX_OPTION_LEN (64)

// Max. length of a generated error message
#define MAX_ERR_LEN (256)

// Store a formatted error string in the error message buffer
#define ARGS_ERR(...) snprintf(_error_message, MAX_ERR_LEN, __VA_ARGS__)


// Buffer for error message
static char _error_message[MAX_ERR_LEN] = {0};

// Generic function for converting a string argument to something else
typedef int (*arg_decoder_t)(char*, void*);

// Functions for converting string arguments to specific types
static int _decode_int(char *input, void *output);
static int _decode_long(char *input, void *output);
static int _decode_uint(char *input, void *output);
static int _decode_ulong(char *input, void *output);
static int _decode_float(char *input, void *output);
static int _decode_double(char *input, void *output);
static int _decode_string(char *input, void *output);
static int _decode_hex(char *input, void *output);

// Structure containing data required to convert an argument to some type
typedef struct {
    arg_decoder_t decode;  // Function to convert string to desired type
    const char *name;      // Human-readable type name
} decode_params_t;


// Decoding table - maps an argtype_e to corresponding decode_params_t object
static decode_params_t _decoders[] = {
    { .decode=_decode_int, .name="integer" },                 // ARGTYPE_INT
    { .decode=_decode_long, .name="long integer" },           // ARGTYPE_LONG
    { .decode=_decode_uint, .name="unsigned integer" },       // ARGTYPE_UINT
    { .decode=_decode_ulong, .name="unsigned long integer" }, // ARGTYPE_ULONG
    { .decode=_decode_float, .name="floating point" },        // ARGTYPE_FLOAT
    { .decode=_decode_double, .name="floating point" },       // ARGTYPE_DOUBLE
    { .decode=_decode_string, .name="string" },               // ARGTYPE_STRING
    { .decode=_decode_hex, .name="hexadecimal" }              // ARGTYPE_HEX
};


/* Decoder functions implementation */

static int _decode_int(char *input, void *output)
{
    char *endptr = NULL;
    long intval = strtol(input, &endptr, 10);
    if (!endptr || *endptr)
    {
        return -1;
    }

    int *int_output = (int *)output;
    *int_output = (int)intval;
    return 0;
}


static int _decode_long(char *input, void *output)
{
    char *endptr = NULL;
    long longval = strtol(input, &endptr, 10);
    if (!endptr || *endptr)
    {
        return -1;
    }

    long *long_output = (long *)output;
    *long_output = longval;
    return 0;
}


static int _decode_uint(char *input, void *output)
{
    char *endptr = NULL;
    unsigned long intval = strtoul(input, &endptr, 10);
    if (!endptr || *endptr)
    {
        return -1;
    }

    unsigned *int_output = (unsigned *)output;
    *int_output = (unsigned)intval;
    return 0;
}


static int _decode_ulong(char *input, void *output)
{
    char *endptr = NULL;
    unsigned long longval = strtoul(input, &endptr, 10);
    if (!endptr || *endptr)
    {
        return -1;
    }

    unsigned long *long_output = (unsigned long *)output;
    *long_output = longval;
    return 0;
}


static int _decode_float(char *input, void *output)
{
    char *endptr = NULL;
    double val = strtod(input, &endptr);
    if (!endptr || *endptr)
    {
        return -1;
    }

    float *float_output = (float *)output;
    *float_output = (float)val;
    return 0;
}


static int _decode_double(char *input, void *output)
{
    char *endptr = NULL;
    double val = strtod(input, &endptr);
    if (!endptr || *endptr)
    {
        return -1;
    }

    double *double_output = (double *)output;
    *double_output = val;
    return 0;
}


static int _decode_string(char *input, void *output)
{
    char **outptr = (char **)output;
    *outptr = input;
    return 0;
}


static int _decode_hex(char *input, void *output)
{
    char *endptr = NULL;
    long val = strtol(input, &endptr, 16);
    if (!endptr || *endptr)
    {
        return -1;
    }

    long *long_output = (long *)output;
    *long_output = val;
    return 0;
}


/*
 * Find the corresponding option data for an option string
 */
static args_option_t *_get_option(args_option_t *options, const char *opt)
{
    for (int i = 0; options[i].opt_type != OPTTYPE_NONE; i++)
    {
        const char *shortf = options[i].short_flag;
        const char *longf = options[i].long_flag;

        if (shortf && (0 == strncmp(shortf, opt, MAX_OPTION_LEN)))
        {
            return &options[i];
        }

        if (longf && (0 == strncmp(options[i].long_flag, opt, MAX_OPTION_LEN)))
        {
            return &options[i];
        }
    }

    return NULL;
}


/*
 * Set all flag data to zero
 */
static void _init_options(args_option_t *options)
{
    for (int i = 0; options[i].opt_type != OPTTYPE_NONE; i++)
    {
        // Set all flags to 0
        if (options[i].data && (OPTTYPE_FLAG == options[i].opt_type))
        {
            int *intptr = (int *)options[i].data;
            *intptr = 0;
        }
    }
}


/*
 * Decode the argument data for a named option
 */
static int _decode_value(args_option_t *opt, char *flag, char *input)
{
    decode_params_t *params = &_decoders[opt->arg_type];
    if (params->decode(input, opt->data) < 0)
    {
        ARGS_ERR("%s value required for %s", params->name, flag);
        return -1;
    }

    return 0;
}


/*
 * Parse a single positional arg
 */
static int _parse_positional(char *arg, args_option_t *options)
{

    // Find the first unseen positional arg entry
    args_option_t *opt = NULL;
    for (int i = 0; options[i].opt_type != OPTTYPE_NONE; i++)
    {
        if ((options[i].opt_type == OPTTYPE_POSITIONAL) && !options[i].seen)
        {
            opt = &options[i];
            break;
        }
    }

    // If no unseen positional args, we're done here
    if (NULL == opt)
    {
        return 0;
    }

    opt->seen = 1;
    if (_decode_value(opt, "positional argument", arg) < 0)
    {
        return -1;
    }

    return 0;
}


/*
 * Count the number of positional arguments in option list
 */
static int _count_positionals(args_option_t *options)
{
    int ret = 0;
    for (int i = 0; options[i].opt_type != OPTTYPE_NONE; i++)
    {
        if (OPTTYPE_POSITIONAL == options[i].opt_type)
        {
            ret += 1;
        }
    }

    return ret;
}


/*
 * Parse all named options and flags
 */
static int _parse_options(int argc, char *argv[], args_option_t *options)
{
    int expected_positionals = _count_positionals(options);
    int seen_positionals = 0;

    for (int i = 1; i < argc; i++)
    {
        if ((argv[i][0] == '-') && argv[i][1])
        {
            // Check if we have a matching option
            args_option_t *opt = _get_option(options, argv[i]);
            if (NULL == opt)
            {
                ARGS_ERR("unknown option '%s'", argv[i]);
                return -1;
            }

            if (opt->seen)
            {
                ARGS_ERR("option '%s/%s' is set more than once",
                         opt->short_flag, opt->long_flag);
                return -1;
            }

            opt->seen = 1;

            // If this is a flag, just set it and we're done
            if (opt->opt_type == OPTTYPE_FLAG)
            {
                int *intptr = (int *)opt->data;
               *intptr = 1;
               continue;
            }

            // Sanity check on data ptr
            if (NULL == opt->data)
            {
                return -1;
            }

            // Sanity check on arg. type
            if (ARRAY_LEN(_decoders) <= opt->arg_type) {
                return -1;
            }

            // Option requires an argument
            if (i == (argc - 1))
            {
                ARGS_ERR("option '%s' requires an argument", argv[i]);
                return -1;
            }

            if (_decode_value(opt, argv[i], argv[i + 1]) < 0)
            {
                return -1;
            }

            i += 1;
        }

        else
        {
            if (seen_positionals >= expected_positionals)
            {
                ARGS_ERR("too many positional arguments");
                return -1;
            }

            if (_parse_positional(argv[i], options) < 0)
            {
                return -1;
            }

            seen_positionals += 1;
        }
    }

    if (seen_positionals < expected_positionals)
    {
        ARGS_ERR("missing positional arguments");
        return -1;
    }

    return 0;
}


char *parse_arguments_error_string(void)
{
    if (0 == _error_message[0])
    {
        return NULL;
    }

    return _error_message;
}


void parse_arguments_print_error(void)
{
    if (0 == _error_message[0])
    {
        return;
    }

    printf("%s\n", _error_message);
}


int parse_arguments(int argc, char *argv[], args_option_t *options)
{
    if ((NULL == argv) || (NULL == options))
    {
        return -1;
    }

    _init_options(options);

    if (_parse_options(argc, argv, options) < 0) {
        return -1;
    }

    return 0;
}

#endif /* PARSE_ARGS_H */
