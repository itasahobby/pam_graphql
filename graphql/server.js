const { ApolloServer, gql } = require('apollo-server');
const { ApolloError } = require('apollo-server-errors');

class InvalidCredentialsError extends ApolloError {
    constructor(message) {
      super(message, 'INVALID_CREDENTIALS');
  
      Object.defineProperty(this, 'name', { value: 'Please provide valid credentials' });
    }
}
  
const typeDefs = gql`
  type Query {
    login(username: String, password: String): String
  }
`;

const resolvers = {
  Query: {
    login: (parent, args, context, info) => {
        let {username, password} = args;
        if (username == "admin" && password == "admin"){
            return 'Logged in successfully';
        } 
        return InvalidCredentialsError;
    },
  },
};


const server = new ApolloServer({ typeDefs, resolvers });
server
  .listen({ port: 9000 })
  .then(({ url }) => console.log(`Server running at ${url}`));
